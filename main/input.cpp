#include "input.hpp"
#include "driver/gpio.h"
#include "esp_timer.h"

struct Btn {
    gpio_num_t pin;
    uint8_t    mask;
    bool       stable;      // estado estable (true = pulsado)
    bool       last_raw;    // última lectura cruda
    int64_t    t_change;    // us del último cambio de lectura cruda
};

static const int64_t DEBOUNCE_US = 15000;  // 15 ms

static Btn s_btns[] = {
    { GPIO_NUM_25, BTN_FEED, false, false, 0 },
    { GPIO_NUM_26, BTN_PLAY, false, false, 0 },
    { GPIO_NUM_27, BTN_MENU, false, false, 0 },
};
static const size_t N_BTN = sizeof(s_btns) / sizeof(s_btns[0]);

void input_init() {
    for (size_t i = 0; i < N_BTN; ++i) {
        gpio_config_t io = {};
        io.pin_bit_mask = 1ULL << s_btns[i].pin;
        io.mode         = GPIO_MODE_INPUT;
        io.pull_up_en   = GPIO_PULLUP_ENABLE;
        io.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io.intr_type    = GPIO_INTR_DISABLE;
        gpio_config(&io);
    }
}

uint8_t input_poll() {
    uint8_t events = BTN_NONE;
    int64_t now = esp_timer_get_time();

    for (size_t i = 0; i < N_BTN; ++i) {
        Btn& b = s_btns[i];
        bool raw = (gpio_get_level(b.pin) == 0);  // activo-bajo

        if (raw != b.last_raw) {
            b.last_raw = raw;
            b.t_change = now;
        } else if ((now - b.t_change) > DEBOUNCE_US && raw != b.stable) {
            b.stable = raw;
            if (b.stable) events |= b.mask;   // flanco de pulsación
        }
    }
    return events;
}
