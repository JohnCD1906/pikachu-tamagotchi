// ============================================================================
//  Configuración de LovyanGFX para un OLED monocromo SSD1306 (128x64) por I2C.
//
//  Pinout (I2C):
//    SDA -> GPIO 21    SCL -> GPIO 22    (VCC 3V3, GND)
//  Dirección I2C típica: 0x3C (algunas placas 0x3D).
//
//  ¿Tu placa es SH1106 en vez de SSD1306? Cambia SOLO la línea del panel:
//    lgfx::Panel_SSD1306 _panel;   ->   lgfx::Panel_SH110x _panel;
//  (el SH1106 suele necesitar offset_x = 2; ver nota abajo).
// ============================================================================
#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// --- Pines / parámetros I2C en un solo sitio ---
#define PIN_SDA   21
#define PIN_SCL   22
#define I2C_ADDR  0x3C
#define I2C_FREQ  400000     // 400 kHz seguro; muchas SSD1306 llegan a 800k-1M

// Geometría del panel
#define OLED_W    128
#define OLED_H     64

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_SSD1306 _panel;   // <- SH1106: cambia por lgfx::Panel_SH110x
    lgfx::Bus_I2C       _bus;

public:
    LGFX() {
        {   // --- Bus I2C ---
            auto cfg = _bus.config();
            cfg.i2c_port    = 0;
            cfg.freq_write  = I2C_FREQ;
            cfg.freq_read   = I2C_FREQ;
            cfg.pin_sda     = PIN_SDA;
            cfg.pin_scl     = PIN_SCL;
            cfg.i2c_addr    = I2C_ADDR;
            _bus.config(cfg);
            _panel.setBus(&_bus);
        }
        {   // --- Panel SSD1306 (monocromo, 1 bit) ---
            auto cfg = _panel.config();
            cfg.panel_width   = OLED_W;
            cfg.panel_height  = OLED_H;
            cfg.memory_width  = OLED_W;
            cfg.memory_height = OLED_H;
            cfg.offset_x      = 0;      // SH1106: pon 2
            cfg.offset_y      = 0;
            _panel.config(cfg);
        }
        setPanel(&_panel);
    }
};
