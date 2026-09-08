#include "sprites.hpp"
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// Corazón 11x10. 1 = encendido, 0 = vacío. Icono genérico de HUD.
const uint8_t sprite_heart[HEART_W * HEART_H] = {
    0,0,1,1,0,0,0,1,1,0,0,
    0,1,1,1,1,0,1,1,1,1,0,
    1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,
    0,1,1,1,1,1,1,1,1,1,0,
    0,0,1,1,1,1,1,1,1,0,0,
    0,0,0,1,1,1,1,1,0,0,0,
    0,0,0,0,1,1,1,0,0,0,0,
    0,0,0,0,0,1,0,0,0,0,0,
};

void draw_bitmap_1bit(lgfx::LGFX_Sprite& g, int x, int y,
                      int w, int h, const uint8_t* data, uint32_t on) {
    for (int j = 0; j < h; ++j)
        for (int i = 0; i < w; ++i)
            if (data[j * w + i]) g.drawPixel(x + i, y + j, on);
}
