// ============================================================================
//  Motor de render.
//  Dibuja TODO el frame en un sprite (framebuffer) de 128x128 en RAM y lo
//  empuja de golpe al panel por DMA -> sin parpadeo.
//  Incluye anti burn-in (desplazamiento lento + salvapantallas al dormir).
// ============================================================================
#pragma once
#include "pet.hpp"

// Crea el display y el framebuffer. Llamar una vez desde la tarea de render.
void render_init();

// Dibuja y empuja un frame. `frame_counter` incrementa 1 por frame; se usa
// para animar y para el desplazamiento anti burn-in.
void render_frame(const PetState& p, uint32_t frame_counter);
