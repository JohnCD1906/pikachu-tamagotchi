// ============================================================================
//  Entrada por 3 botones (activo-bajo, con pull-up interno).
//    BTN_FEED -> GPIO 25    BTN_PLAY -> GPIO 26    BTN_MENU -> GPIO 27
//  Devuelve flancos de pulsación ya con antirrebote. Poll desde la tarea
//  de lógica cada ~15-20 ms.
// ============================================================================
#pragma once
#include <stdint.h>

enum ButtonEvent : uint8_t {
    BTN_NONE = 0,
    BTN_FEED = 1 << 0,
    BTN_PLAY = 1 << 1,
    BTN_MENU = 1 << 2,   // pulsación corta = dormir/despertar
};

// Configura los GPIO. Llamar una vez.
void input_init();

// Devuelve un OR de los ButtonEvent que se hayan "pulsado" desde la última
// llamada (flanco de bajada ya filtrado). Llamar periódicamente.
uint8_t input_poll();
