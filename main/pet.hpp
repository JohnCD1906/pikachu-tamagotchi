// ============================================================================
//  Estado de la mascota y su lógica de decaimiento en el tiempo.
//  Todas las stats van de 0 a 100.
// ============================================================================
#pragma once
#include <stdint.h>
#include <time.h>

enum class PetMood : uint8_t {
    IDLE,      // tranquilo
    HAPPY,     // acaba de jugar / feliz
    EATING,    // comiendo
    SLEEPING,  // durmiendo (sin energía o el usuario lo durmió)
    SICK,      // salud baja
    DEAD       // se acabó
};

struct PetState {
    uint8_t hunger;       // 100 = lleno, baja con el tiempo
    uint8_t happiness;    // 100 = feliz
    uint8_t energy;       // 100 = descansado
    uint8_t health;       // 100 = sano
    uint32_t age_seconds; // edad acumulada (segundos de vida)

    time_t   last_update; // marca de tiempo (RTC interno) del último decaimiento
    PetMood  mood;        // estado de ánimo derivado (para el render)
    bool     forced_sleep;// el usuario lo mandó a dormir manualmente

    // Acumuladores internos para no perder el resto al decaer 1 punto cada N s.
    // No es crítico persistirlos con exactitud.
    uint16_t acc_hunger;
    uint16_t acc_happy;
    uint16_t acc_energy;
};

// Valores de una mascota recién nacida.
void    pet_init_defaults(PetState& p);

// Aplica el efecto de `seconds` segundos transcurridos sobre las stats.
void    pet_apply_elapsed(PetState& p, uint32_t seconds);

// Acciones del usuario.
void    pet_feed(PetState& p);
void    pet_play(PetState& p);
void    pet_toggle_sleep(PetState& p);

// Recalcula el mood a partir de las stats. Lo llama pet_apply_elapsed,
// pero se expone por si quieres refrescarlo tras una acción.
PetMood pet_compute_mood(const PetState& p);
