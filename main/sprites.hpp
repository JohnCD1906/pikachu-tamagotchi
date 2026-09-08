// ============================================================================
//  Sprites (MONOCROMO, para OLED SSD1306 128x64).
//
//  NO CONTIENE ARTE DE PIKACHU (personaje con derechos). Trae:
//    1) Un icono de corazón 1-bit como EJEMPLO del formato: un array de bytes
//       donde 1 = píxel encendido, 0 = apagado (se omite).
//    2) draw_bitmap_1bit(): blitea uno de estos bitmaps en el framebuffer.
//    3) El motor dibuja un "monstruito" placeholder de forma procedural para
//       que veas todo funcionando ya. Reemplázalo por tu pixel art.
//
//  CÓMO METER TU PIXEL ART (monocromo):
//    - Dibuja cada frame en blanco y negro (p. ej. 40x40).
//    - Conviértelo a un array 1-bit (1 = pixel encendido). Sirve el mismo
//      script de Python del README pero con un umbral de brillo.
//    - Blitéalo con draw_bitmap_1bit() donde hoy va draw_placeholder_creature().
// ============================================================================
#pragma once
#include <stdint.h>

namespace lgfx { inline namespace v1 { class LGFX_Sprite; } }

static const int HEART_W = 11;
static const int HEART_H = 10;

// Icono de ejemplo (corazón), 1 byte por pixel: 1 = encendido, 0 = vacío.
extern const uint8_t sprite_heart[HEART_W * HEART_H];

// Dibuja un bitmap 1-bit en (x,y). Solo pinta los píxeles con valor 1,
// con el color `on` (deja el fondo intacto -> transparencia gratis).
void draw_bitmap_1bit(lgfx::LGFX_Sprite& g, int x, int y,
                      int w, int h, const uint8_t* data, uint32_t on);
