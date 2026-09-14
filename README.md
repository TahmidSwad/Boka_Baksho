# BoomBox2 — Lightweight Multi-App Framework (ESP32)

A small multi-application OS-style framework for the **Boom Box** device:
an ESP32 with a 128x64 SSD1306 OLED, a **rotary encoder**, and two
buttons (ENTER / BACK). Includes a **SystemUi launcher** and a karaoke
**Lyrics app**.

```
┌─────────────────────────────────────────────────────┐
│  App layer    apps/lyrics, apps/clock (per-app UI +  │
│                own interpretation of input events)   │
│  SystemUi     launcher: scroll apps, enter/back      │
│  Core         AppManager (registry + nav stack),     │
│               EventBus (display / input events)      │
│  Services     DisplayService (foreground-only draw), │
│               InputService (encoder + buttons),      │
│               StorageService (LittleFS), Audio stub  │
│  Drivers      Oled (U8g2), RotaryEncoder, Button     │
└─────────────────────────────────────────────────────┘
```

## Architecture

- **Navigation stack** — the SystemUi launcher sits at the bottom of the
  stack. Launching an app pushes it; BACK pops it. There is always a
  foreground app.
- **Input routing** — the encoder and buttons are normalized into
  `InputEvent`s (`RotateLeft`, `RotateRight`, `Enter`, `Back`) and
  delivered to the foreground app. If an app does **not** consume a
  `Back` event, the AppManager pops the stack back to the launcher.
- **Display ownership** — the DisplayService renders a request only when
  its sender is the current foreground app (no ResourceManager needed).
- **Lightweight** — zero dynamic allocation; static arrays only
  (registry = `AppId::Count`, nav stack = 8, event queue = 16).

## Controls

| Hardware       | In launcher              | In an app (examples)        |
|----------------|--------------------------|-----------------------------|
| encoder rotate | scroll app list          | Lyrics: switch lyric file   |
| ENTER button   | open selected app       | Lyrics: play / pause        |
| BACK button    | (nothing, root menu)     | exit app -> launcher        |

Serial debug input (no hardware needed): `l`/`r`/`e`/`b` post
`RotateLeft` / `RotateRight` / `Enter` / `Back` events.

## Layout

```
BoomBox2/
├── platformio.ini
├── data/lyrics/           Tumi.txt, Closer.txt (timestamped lyrics)
└── src/
    ├── main.cpp           boot, registration, main loop
    ├── common/            config, event/input/ui types
    ├── core/              app_base, app_manager, event_bus, system
    ├── drivers/           oled, rotary encoder, button
    ├── services/          display, input, storage, audio (stub)
    ├── system_ui/         launcher app
    └── apps/              lyrics, clock (+ future music/alarm/settings)
```

## Build / upload

```sh
pio run                     # compile
pio run -t uploadfs         # upload data/ (littlefs)
pio run -t upload           # flash firmware
pio device monitor          # serial: logs + debug input l/r/e/b
```

Pins (see `src/common/config.h`): encoder CLK=25, DT=26, ENTER=27, BACK=14;
OLED I2C SDA=21, SCL=22. At boot the OLED driver probes I2C address
`0x3C`, then falls back to `0x3D`, and prints `OLED I2C: device at
0x..` (or `OLED I2C: FAILED ...`) on serial.