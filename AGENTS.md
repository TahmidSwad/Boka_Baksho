# AGENTS.md — BoomBox2 Project Context

## Project Overview

BoomBox2 is a lightweight multi-application OS-style framework for an ESP32-based portable device ("Boom Box") with a 128x64 SSD1306 OLED display, four physical buttons (Increment, Decrement, ENTER, BACK), BLE connectivity, and LittleFS storage. The device runs a karaoke lyrics synchronization system and a stopwatch/timer app, with a scrollable launcher UI.

**Platform:** ESP32 (Arduino framework, PlatformIO build system)
**Target board:** `esp32doit-devkit-v1`
**Filesystem:** LittleFS

## Architecture

See `docs/architecture.md` for the full layered architecture description. In summary:

- **Drivers** (Oled, Button) — hardware abstraction, no domain logic.
- **Services** (DisplayService, InputService, StorageService, BleService, AudioService) — system-level concerns, subscribe to EventBus.
- **Core** (AppManager, EventBus, IApp interface) — app lifecycle, navigation stack, event routing.
- **SystemUI** — the root launcher app (always at bottom of nav stack).
- **Apps** (LyricsApp, StopwatchApp) — user-facing applications implementing `IApp`.

Data flows through a central `EventBus` (ring buffer, pub/sub). All objects are statically allocated globals.

## Important Files

| File | Purpose |
|------|---------|
| `platformio.ini` | Build config, board, filesystem, dependencies |
| `src/main.cpp` | Boot sequence, registration, main loop |
| `src/common/config.h` | Hardware pins, I2C addresses, file paths |
| `src/common/event_types.h` | AppId, EventType, Event struct, DisplayRequest |
| `src/common/input_types.h` | InputType enum, InputEvent struct |
| `src/common/ui_types.h` | TextSize, TextAlign, DisplayRequestType enums |
| `src/core/app_base.h` | IApp interface (Begin/OnActivate/OnDeactivate/Update/HandleInput) |
| `src/core/app_manager.h/.cpp` | Registry + navigation stack + input routing |
| `src/core/event_bus.h/.cpp` | Ring-buffer event queue + typed subscriber lists |
| `src/core/system.h` | Extern declarations for global singletons |
| `src/drivers/oled/oled.h/.cpp` | U8g2 SSD1306 driver with I2C probe |
| `src/drivers/button/button.h/.cpp` | Debounced active-low button driver |
| `src/services/display_service.h/.cpp` | Renders DisplayRequest events, enforces foreground ownership |
| `src/services/input_service.h/.cpp` | Polls buttons + serial debug, posts InputEvents |
| `src/services/storage_service.h/.cpp` | LittleFS wrapper |
| `src/services/ble_service.h/.cpp` | BLE GATT server for Python client communication |
| `src/services/audio_service.h/.cpp` | Audio playback stub (no implementation) |
| `src/system_ui/system_ui.h/.cpp` | Launcher app (scroll menu, launch selection) |
| `src/apps/lyrics/lyrics_app.h/.cpp` | Karaoke lyrics app (file list, parse, BLE sync) |
| `src/apps/clock/stopwatch_app.h/.cpp` | Stopwatch/timer app |
| `data/lyrics/` | Timestamped lyric files (Tumi.txt, Closer.txt) |

## Coding Conventions

- **Language:** C++ (Arduino-flavored, no STL, no dynamic allocation).
- **Namespaces:** `config::pins`, `config::display` for hardware constants.
- **Classes:** PascalCase for class names, PascalCase for public methods, snake_case_ trailing underscore for private members.
- **Enums:** Scoped (`enum class`), PascalCase values.
- **Globals:** Declared `extern` in headers, defined once in `.cpp` files. Named in snake_case (e.g., `app_manager`, `event_bus`, `oled`).
- **Includes:** Use quoted project-relative paths (e.g., `#include "core/system.h"`). System/Arduino includes use angle brackets.
- **No comments unless asked.** The codebase is intentionally comment-light.
- **Static arrays only.** Never use `new`, `malloc`, `std::vector`, or any dynamic allocation.
- **File organization:** Each class gets its own `.h`/`.cpp` pair, placed in the appropriate subdirectory.

## Constraints

- **Static allocation only.** All data structures are compile-time sized. Registry size = `AppId::Count` (6), nav stack depth = 8, event queue = 16, subscribers per type = 4.
- **Single foreground owner.** Display ownership is enforced by `DisplayService` — only the app at the top of the nav stack can draw.
- **No resource manager.** Display exclusivity is implicit via the nav stack, not a separate resource tracking system.
- **OLED I2C probe at boot.** The driver probes `0x3C` then `0x3D` before initializing. Serial output indicates success/failure.
- **BLE is a text protocol.** Newline-terminated plain text commands over GATT TX/RX characteristics.

## Workflow Rules

1. **At the start of every iteration:** Read this file (`AGENTS.md`), then read the relevant sections of `docs/architecture.md`, `docs/decisions.md`, and `docs/current-state.md`. Then inspect only the source files relevant to the current task. Do NOT read the entire repository if context files identify the relevant files.

2. **After making code changes:** Update `docs/current-state.md`, `docs/change-log.md`, and if applicable `docs/decisions.md` and `docs/architecture.md`. Only update `AGENTS.md` if a persistent project-wide rule has changed.

3. **Build verification:** After making code changes, attempt to compile with `pio run` (or the equivalent build script) if the build environment is available. The build command is defined in `platformio.ini`.

4. **Adding a new app:** Add an `AppId` value in `src/common/event_types.h` (before `Count`), create the app class implementing `IApp`, register it in `main.cpp` via `app_manager.RegisterApp()`, and call its `Begin()`.

5. **Adding a new service:** Create the service class, define a global instance, and wire it into the EventBus subscription list in `main.cpp` if it needs to receive events.

6. **Never claim a feature is implemented if it is only partially implemented or unverified.**
