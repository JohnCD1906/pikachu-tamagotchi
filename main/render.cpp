#include "render.hpp"
#include "sprites.hpp"
#include "lgfx_config.hpp"
#include <math.h>

static LGFX               s_lcd;             // OLED SSD1306 128x64
static lgfx::LGFX_Sprite  s_fb(&s_lcd);      // framebuffer 1-bit (128*64/8 = 1 KB)

static const int W = OLED_W;   // 128
static const int H = OLED_H;   // 64

// En 1 bit: 1 = píxel encendido, 0 = apagado.
static const uint32_t ON  = 1;
static const uint32_t OFF = 0;

void render_init() {
    s_lcd.init();
    s_lcd.setRotation(0);
    s_lcd.setBrightness(255);
    s_fb.setColorDepth(1);          // monocromo
    s_fb.createSprite(W, H);        // ~1 KB en RAM
}

// ---------------------------------------------------------------------------
//  Placeholder procedural: "monstruito" genérico (NO es Pikachu).
//  Blob blanco con cara oscura, que rebota y parpadea según el estado.
//  Ocupa la mitad izquierda; reemplázalo por tu pixel art (ver sprites.hpp).
// ---------------------------------------------------------------------------
static void draw_placeholder_creature(lgfx::LGFX_Sprite& g, const PetState& p,
                                      uint32_t frame, int cx, int cy) {
    int bob = 0;
    if (p.mood == PetMood::IDLE || p.mood == PetMood::HAPPY || p.mood == PetMood::EATING)
        bob = (int)lroundf(2.0f * sinf(frame * 0.18f));
    cy += bob;

    int r = 17;
    g.fillCircle(cx, cy, r, ON);              // cuerpo (blanco)
    g.fillCircle(cx - 9, cy + r - 3, 4, ON);  // piececitos
    g.fillCircle(cx + 9, cy + r - 3, 4, ON);

    int ex = 7, ey = -3;
    bool blink = ((frame / 6) % 20 == 0);

    if (p.mood == PetMood::SLEEPING) {
        g.drawFastHLine(cx - ex - 3, cy + ey, 6, OFF);   // ojos cerrados
        g.drawFastHLine(cx + ex - 3, cy + ey, 6, OFF);
        g.setTextColor(ON);
        g.setCursor(cx + r - 2, cy - r - 2);
        g.print("z");
    } else if (p.mood == PetMood::DEAD) {
        g.drawLine(cx - ex - 3, cy + ey - 3, cx - ex + 3, cy + ey + 3, OFF);
        g.drawLine(cx - ex + 3, cy + ey - 3, cx - ex - 3, cy + ey + 3, OFF);
        g.drawLine(cx + ex - 3, cy + ey - 3, cx + ex + 3, cy + ey + 3, OFF);
        g.drawLine(cx + ex + 3, cy + ey - 3, cx + ex - 3, cy + ey + 3, OFF);
    } else if (blink) {
        g.drawFastHLine(cx - ex - 2, cy + ey, 4, OFF);
        g.drawFastHLine(cx + ex - 2, cy + ey, 4, OFF);
    } else {
        g.fillCircle(cx - ex, cy + ey, 3, OFF);   // ojos oscuros
        g.fillCircle(cx + ex, cy + ey, 3, OFF);
        g.drawPixel(cx - ex + 1, cy + ey - 1, ON);// brillito
        g.drawPixel(cx + ex + 1, cy + ey - 1, ON);
    }

    // Boca (oscura) según estado
    int my = cy + 8;
    if (p.mood == PetMood::HAPPY) {
        for (int i = -5; i <= 5; ++i)
            g.drawPixel(cx + i, my + (int)lroundf(3 - (i * i) / 9.0f), OFF);
    } else if (p.mood == PetMood::EATING) {
        g.fillCircle(cx, my + 1, 3, OFF);
    } else if (p.mood == PetMood::SICK) {
        for (int i = -5; i <= 5; ++i)
            g.drawPixel(cx + i, my - (int)lroundf(3 - (i * i) / 9.0f) + 3, OFF);
    } else {
        g.drawFastHLine(cx - 4, my + 2, 8, OFF);
    }
}

// ---------------------------------------------------------------------------
//  HUD: 4 barras etiquetadas en la mitad derecha + corazón/edad arriba.
// ---------------------------------------------------------------------------
static void draw_labeled_bar(lgfx::LGFX_Sprite& g, int x, int y,
                             char label, uint8_t val) {
    g.setTextColor(ON);
    g.setCursor(x, y - 1);
    g.print(label);                 // letra (H/F/E/S)
    int bx = x + 8, bw = 44, bh = 7;
    g.drawRect(bx, y, bw, bh, ON);  // marco
    int fill = (val * (bw - 2)) / 100;
    g.fillRect(bx + 1, y + 1, fill, bh - 2, ON);
}

static void draw_hud(lgfx::LGFX_Sprite& g, const PetState& p) {
    // Corazón + edad (minutos) arriba a la izquierda.
    draw_bitmap_1bit(g, 2, 2, HEART_W, HEART_H, sprite_heart, ON);
    g.setTextColor(ON);
    g.setCursor(15, 3);
    g.printf("%lum", (unsigned long)(p.age_seconds / 60));

    // 4 barras a la derecha (H=hambre, F=felicidad, E=energia, S=salud).
    int x0 = 72;
    draw_labeled_bar(g, x0,  2, 'H', p.hunger);
    draw_labeled_bar(g, x0, 17, 'F', p.happiness);
    draw_labeled_bar(g, x0, 32, 'E', p.energy);
    draw_labeled_bar(g, x0, 47, 'S', p.health);
}

void render_frame(const PetState& p, uint32_t frame) {
    // Anti burn-in: desplazamiento lento de -1..+1 px (el OLED tambien se quema).
    int ox = (int)((frame / 900) % 3) - 1;
    int oy = (int)((frame / 1800) % 3) - 1;

    bool screensaver = (p.mood == PetMood::SLEEPING);

    s_fb.fillScreen(OFF);
    // La criatura vive en la mitad izquierda; el HUD en la derecha.
    draw_placeholder_creature(s_fb, p, frame, 36 + ox, 36 + oy);
    draw_hud(s_fb, p);

    s_fb.pushSprite(0, 0);

    // Atenua el contraste del OLED al dormir (comando de contraste del SSD1306).
    s_lcd.setBrightness(screensaver ? 30 : 255);
}
