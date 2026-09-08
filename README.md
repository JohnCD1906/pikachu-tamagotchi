# Tamagotchi ESP32 + OLED SSD1306 (monocromo)

Firmware para un tamagotchi en un **ESP32 DevKit** con un **OLED monocromo
SSD1306 128×64 por I2C**, usando **LovyanGFX**, **RTC interno**, persistencia en
**NVS** y reparto de trabajo en los **dos núcleos** con FreeRTOS.

> **Nota sobre el arte:** este proyecto **no incluye sprites de Pikachu** (es un
> personaje con derechos). Trae un *placeholder* genérico animado y el pipeline
> listo para que pegues tu propio pixel art monocromo. Ver "Añadir tus sprites".

---

## Cableado

### OLED SSD1306 (I2C)

| OLED   | ESP32   | Definido en       |
|--------|---------|-------------------|
| VCC    | 3V3     | —                 |
| GND    | GND     | —                 |
| SDA    | GPIO 21 | `lgfx_config.hpp` |
| SCL    | GPIO 22 | `lgfx_config.hpp` |

Dirección I2C por defecto **0x3C** (algunas placas 0x3D). Si tu panel es
**SH1106** en vez de SSD1306, cambia solo la línea del panel en `lgfx_config.hpp`
(`Panel_SSD1306` → `Panel_SH110x`) y pon `offset_x = 2`.

### Botones (activo-bajo, pull-up interno; otro extremo a GND)

| Botón     | ESP32   | Acción                        |
|-----------|---------|-------------------------------|
| Alimentar | GPIO 25 | sube hambre                   |
| Jugar     | GPIO 26 | sube felicidad, gasta energía |
| Menú      | GPIO 27 | dormir / despertar            |

Evita GPIO 6–11 (flash) y ojo con los strapping (0, 2, 12, 15).

---

## Alimentación (con lo que tienes)

Celda **LiPo 3.7 V 1000 mAh** + **TP4056 con protección** (carga y protege la
celda) + un **convertidor step-down (buck)** para dar 3.3 V al DevKit.

```
 USB ─► [TP4056 + protección] ─► BAT(3.0–4.2V) ─► [BUCK a 3.3V] ─► pin 3V3 del DevKit
                                                                    (GND común)
```

Reglas al conectar el buck:

1. **Ajusta la salida a 3.30 V con el multímetro ANTES de tocar el DevKit**
   (en vacío, gira el potenciómetro).
2. Va al **pin 3V3**, NO a VIN (un buck no puede subir a 5 V).
3. **No lo alimentes por USB del DevKit al mismo tiempo** que por el 3V3 (el
   AMS1117 pelearía con tu buck). Para cargar, usa el USB del TP4056 aparte.

Limitación: cuando la celda baje de ~3.5 V el buck deja de mantener 3.3 V y el
aparato se apaga (aprovechas ~75% de la celda). Un **buck-boost** lo resolvería
más adelante si quieres exprimir toda la batería.

---

## Compilar y flashear (ESP-IDF ≥ 5.0)

```bash
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

LovyanGFX se descarga solo en el primer `build` (ver `main/idf_component.yml`).

---

## Arquitectura (dos núcleos)

```
        ┌──────────────── Core 0 ─────────────────┐
        │  logic_task                             │
        │   • input_poll() (botones, antirrebote) │
        │   • pet_apply_elapsed() cada 1 s (RTC)  │
        │   • storage_save() cada 30 s  (NVS)     │ ← los bloqueos de flash viven aquí
        └───────────────┬─────────────────────────┘
                        │  g_pet  (mutex)
        ┌───────────────▼──────── Core 1 ─────────┐
        │  render_task                            │
        │   • copia g_pet                         │
        │   • dibuja framebuffer 128×64 (1 bit)   │
        │   • pushSprite() al OLED por I2C        │ ← animación siempre fluida
        └─────────────────────────────────────────┘
```

El framebuffer monocromo es de solo **1 KB** (128×64/8), así que incluso por I2C
a 400 kHz mueves ~25–30 fps de sobra. Puedes subir `I2C_FREQ` a 800 k–1 M si tu
placa lo aguanta.

---

## Estructura del código

| Archivo            | Qué hace                                                    |
|--------------------|-------------------------------------------------------------|
| `main.cpp`         | app_main, RTC, y las dos tareas pineadas a cada núcleo       |
| `lgfx_config.hpp`  | Config de LovyanGFX para el SSD1306 por I2C (pines, addr)    |
| `pet.hpp/.cpp`     | Struct de la mascota y decaimiento de stats por tiempo       |
| `storage.hpp/.cpp` | Guardar/cargar la mascota en NVS                             |
| `input.hpp/.cpp`   | 3 botones con antirrebote                                    |
| `render.hpp/.cpp`  | Motor de render monocromo, HUD y anti burn-in                |
| `sprites.hpp/.cpp` | Icono de ejemplo 1-bit + blitter; aquí van tus bitmaps       |

Layout de pantalla: la criatura ocupa la mitad izquierda; a la derecha, 4 barras
etiquetadas **H**ambre / **F**elicidad / **E**nergía / **S**alud; arriba a la
izquierda, corazón + edad en minutos.

---

## Añadir tus sprites (monocromo)

1. Dibuja cada frame en blanco y negro (p. ej. 40×40).
2. Conviértelo a un array 1-bit (1 = píxel encendido) con un umbral de brillo:

   ```python
   from PIL import Image
   img = Image.open("idle0.png").convert("L")   # escala de grises
   w, h = img.size
   vals = [1 if img.getpixel((x, y)) > 127 else 0
           for y in range(h) for x in range(w)]
   print(f"static const uint8_t idle0[{w*h}] = {{" +
         ",".join(map(str, vals)) + "};")
   ```

3. Pega los arrays en `sprites.cpp` / decláralos en `sprites.hpp`.
4. En `render.cpp`, cambia `draw_placeholder_creature(...)` por:

   ```cpp
   draw_bitmap_1bit(s_fb, cx - W/2, cy - H/2, W, H, tu_frame, 1);
   ```

---

## Ajustes rápidos

- **Dificultad:** ritmos de decaimiento en `pet.cpp` (`HUNGER_EVERY`, etc.).
- **Dirección I2C / velocidad:** `I2C_ADDR`, `I2C_FREQ` en `lgfx_config.hpp`.
- **Nada aparece / pantalla en blanco:** casi siempre es la dirección I2C (0x3C
  vs 0x3D) o SDA/SCL cruzados. Un escáner I2C confirma la dirección.
- **FPS:** el `period` de `render_task` en `main.cpp` (33 ms ≈ 30 fps).
- **Anti burn-in:** función `render_frame` (desplazamiento) y el modo dormir.

---

## Limitación conocida del RTC interno

El RTC del ESP32 mantiene la hora en **deep-sleep**, así que la mascota envejece
mientras duerme. Pero si se **corta toda la alimentación**, el reloj se reinicia
y no se puede medir el hueco offline (el firmware lo detecta y no la envejece de
golpe). Para tiempo real exacto tras un apagado total, habría que añadir un
**DS3231** por I2C (compartiría el mismo bus que el OLED).
