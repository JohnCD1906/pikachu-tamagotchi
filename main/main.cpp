// ============================================================================
//  Tamagotchi — punto de entrada y reparto en los dos núcleos del ESP32.
//
//    Core 0 (logic_task)  : botones, decaimiento por tiempo real (RTC interno),
//                           guardado en NVS.  <- aquí pasan los bloqueos de flash
//    Core 1 (render_task) : construye el framebuffer y lo empuja al OLED por I2C.
//                           <- animación siempre fluida, nunca se frena por NVS
//
//  Estado compartido: g_pet, protegido por un mutex. La lógica escribe, el
//  render solo lee una copia. Simple y sin sustos.
// ============================================================================
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <time.h>
#include <sys/time.h>

#include "pet.hpp"
#include "storage.hpp"
#include "input.hpp"
#include "render.hpp"

static const char* TAG = "tama";

// Estado compartido entre los dos núcleos.
static PetState          g_pet;
static SemaphoreHandle_t g_mutex;

// Época base para sembrar el RTC interno en el primer arranque (2024-01-01).
static const time_t TIME_BASE = 1704067200;

// ---------------------------------------------------------------------------
//  Tarea de lógica (core 0)
// ---------------------------------------------------------------------------
static void logic_task(void*) {
    input_init();

    const TickType_t period = pdMS_TO_TICKS(20);   // poll ~50 Hz (botones fluidos)
    int64_t  last_save    = esp_timer_get_time();
    int64_t  action_until = 0;
    PetMood  action_mood  = PetMood::IDLE;
    time_t   last_sec     = time(NULL);

    for (;;) {
        uint8_t ev     = input_poll();
        int64_t now_us = esp_timer_get_time();

        xSemaphoreTake(g_mutex, portMAX_DELAY);

        if (ev & BTN_FEED) { pet_feed(g_pet); action_mood = PetMood::EATING; action_until = now_us + 2000000; }
        if (ev & BTN_PLAY) { pet_play(g_pet); action_mood = PetMood::HAPPY;  action_until = now_us + 2000000; }
        if (ev & BTN_MENU) { pet_toggle_sleep(g_pet); }

        // Decaimiento 1 vez por segundo, medido con el RTC interno.
        time_t nowt = time(NULL);
        if (nowt > last_sec) {
            uint32_t elapsed = (uint32_t)(nowt - last_sec);
            if (elapsed > 3600) elapsed = 1;   // salto raro -> no envejecer de golpe
            pet_apply_elapsed(g_pet, elapsed);
            g_pet.last_update = nowt;
            last_sec = nowt;
        } else if (nowt < last_sec) {
            last_sec = nowt;                   // el reloj retrocedió (corte de luz): resync
        }

        // Mood transitorio tras una acción (comer/jugar) durante ~2 s.
        if (now_us < action_until && g_pet.mood != PetMood::DEAD)
            g_pet.mood = action_mood;

        xSemaphoreGive(g_mutex);

        // Guardado periódico en NVS (aquí, en core 0, para no frenar el render).
        if (now_us - last_save > 30000000) {   // cada 30 s
            PetState snap;
            xSemaphoreTake(g_mutex, portMAX_DELAY);
            snap = g_pet;
            xSemaphoreGive(g_mutex);
            storage_save(snap);
            last_save = now_us;
        }

        vTaskDelay(period);
    }
}

// ---------------------------------------------------------------------------
//  Tarea de render (core 1)
// ---------------------------------------------------------------------------
static void render_task(void*) {
    render_init();                              // crea display + framebuffer (~1 KB, 1-bit)

    const TickType_t period = pdMS_TO_TICKS(33);// ~30 fps
    uint32_t frame = 0;
    PetState snap;

    for (;;) {
        xSemaphoreTake(g_mutex, portMAX_DELAY);
        snap = g_pet;                           // copia rápida bajo mutex
        xSemaphoreGive(g_mutex);

        render_frame(snap, frame++);
        vTaskDelay(period);
    }
}

// ---------------------------------------------------------------------------
//  app_main
// ---------------------------------------------------------------------------
extern "C" void app_main(void) {
    storage_init();

    // Sembrar el RTC interno solo si aún no tiene hora válida (primer boot).
    time_t now = time(NULL);
    if (now < TIME_BASE) {
        struct timeval tv = { .tv_sec = TIME_BASE, .tv_usec = 0 };
        settimeofday(&tv, NULL);
        now = TIME_BASE;
    }

    // Cargar mascota y "ponerla al día" por el tiempo transcurrido.
    bool had_save = storage_load(g_pet);
    if (had_save && g_pet.last_update > 0 && now > g_pet.last_update) {
        uint32_t gap = (uint32_t)(now - g_pet.last_update);
        if (gap > 7 * 24 * 3600) gap = 7 * 24 * 3600;   // tope: 7 días
        ESP_LOGI(TAG, "Poniendo al dia %lu s fuera de linea", (unsigned long)gap);
        pet_apply_elapsed(g_pet, gap);
    }
    g_pet.last_update = now;

    g_mutex = xSemaphoreCreateMutex();

    // Lógica en el core 0, render en el core 1.
    xTaskCreatePinnedToCore(logic_task,  "logic",  4096, nullptr, 5, nullptr, 0);
    xTaskCreatePinnedToCore(render_task, "render", 8192, nullptr, 5, nullptr, 1);

    ESP_LOGI(TAG, "Tamagotchi arrancado");
}
