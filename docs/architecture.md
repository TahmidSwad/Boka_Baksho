# BoomBox2 Architecture

## Overview

BoomBox2 is a layered OS-style framework for an ESP32 portable device. It provides a scrollable app launcher, an event-driven core, and a clean separation between hardware drivers, system services, and user-facing applications.

```
┌─────────────────────────────────────────────────────────┐
│  App Layer     LyricsApp, StopwatchApp                  │
│                (each owns a state machine + UI)         │
├─────────────────────────────────────────────────────────┤
│  SystemUI      Launcher: scroll menu, select to launch  │
├─────────────────────────────────────────────────────────┤
│  Core          AppManager   (registry + nav stack)      │
│                EventBus     (ring-buffer pub/sub)       │
│                IApp         (app interface contract)     │
├─────────────────────────────────────────────────────────┤
│  Services      DisplayService  (foreground-only draw)   │
│                InputService    (button polling + serial) │
│                StorageService  (LittleFS wrapper)       │
│                BleService      (GATT text protocol)     │
│                AudioService    (stub, unimplemented)    │
├─────────────────────────────────────────────────────────┤
│  Drivers       Oled   (U8g2 SSD1306 I2C)               │
│                Button (debounced active-low polling)    │
├─────────────────────────────────────────────────────────┤
│  Common        config, event_types, input_types, ui_types│
└─────────────────────────────────────────────────────────┘
```

All objects are statically allocated globals. No `new`, `malloc`, or STL containers are used anywhere.

## Data Flow

### Input Path

```
Hardware Buttons  ──>  Button::Poll()  ──>  InputService::Poll()
                                                  │
                                                  ▼
                                          EventBus.Post(InputEvent)
                                                  │
                                                  ▼
                                          AppManager::OnEvent()
                                                  │
                                                  ▼
                                          RouteInput() -> foreground app
```

`InputService` also polls serial (`PollSerialDebug`) for `l/r/e/b/+/-` characters, posting the same `InputEvent` types — enabling full system testing without hardware.

### Display Path

```
App calls  ──>  EventBus.Post(DisplayRequest)  ──>  DisplayService::OnEvent()
                                                          │
                                                          ▼
                                                  Ownership check:
                                                  sender == foreground app?
                                                          │
                                                  yes ────┤
                                                          ▼
                                                  Render on OLED
```

Display ownership is enforced in `DisplayService::OnEvent()`: only the app currently at the top of the navigation stack (`app_manager.GetForeground()`) may draw. This replaces a hypothetical ResourceManager with a simpler implicit check.

### Event Bus Internals

The `EventBus` (`src/core/event_bus.h/.cpp`) is a fixed-size ring buffer (`kEventQueueSize = 16`). Subscribers register per `EventType` (max 4 per type). `Dispatch()` drains the queue and invokes each subscriber's `OnEvent()`.

```
EventBus
  ├── subscribers_[EventType::Count][4]   typed subscriber arrays
  ├── queue_[16]                          ring buffer of Event structs
  └── Dispatch()                          drains queue, calls subscribers
```

**Critical constraint:** `EventBus::Post()` copies the `Event` struct into the queue, but any **pointer fields** inside the event (e.g., `DisplayRequest::lines`, `DisplayRequest::text`) are shallow-copied. The pointed-to data must remain valid until `Dispatch()` processes the event. Use **member variables** (not stack locals) for any data referenced by queued events. Stack-local arrays become dangling pointers as soon as the posting function returns.

## Navigation Stack

`AppManager` (`src/core/app_manager.h/.cpp`) maintains:

- **Registry** (`apps_[6]`) — all registered apps (size = `AppId::Count`).
- **Navigation stack** (`stack_[8]`) — depth 1..8. Bottom is always `SystemUi`.

| Operation | Effect |
|-----------|--------|
| `Boot(system_ui)` | Sets `stack_[0] = system_ui`, calls `OnActivate()` |
| `LaunchApp(id)` | Deactivates current top, pushes target, activates it |
| `GoBack()` | Deactivates top, pops, activates new top. Never pops root. |

Input routing: `AppManager::OnEvent()` calls `RouteInput()` which forwards to the foreground app's `HandleInput()`. If the app returns `false` for a `Back` event, `GoBack()` is called automatically.

## App Lifecycle (`IApp`)

Every app implements the `IApp` interface (`src/core/app_base.h`):

| Method | When Called |
|--------|------------|
| `Begin()` | Once at system startup (after registration) |
| `OnActivate()` | App becomes the top of the nav stack (visible, receives input) |
| `OnDeactivate()` | App leaves the top of the stack (hidden) |
| `Update()` | Every `loop()` iteration while the app is on top |
| `HandleInput(event)` | When input is routed to this app. Return `true` if consumed. |

Apps post `DisplayRequest` events to the `EventBus` to draw. They do not touch the `Oled` driver directly.

## BLE Protocol

`BleService` (`src/services/ble_service.h/.cpp`) implements a GATT server with:

- **TX characteristic** (NOTIFY) — ESP32 -> Python client
- **RX characteristic** (WRITE) — Python client -> ESP32

Protocol is newline-terminated plain text:

| Direction | Commands |
|-----------|----------|
| ESP32 -> Python | `PLAY\|song_name`, `STOP`, `PAUSE`, `RESUME`, `TIME_SYNC\|<ms>` |
| Python -> ESP32 | `AUDIO_STARTED`, `STOPPED`, `PAUSED`, `RESUMED`, `TIME_ACK\|<ms>` |

Incoming commands are buffered in `rx_buffer_[256]` and processed line-by-line in `ProcessLine()`. A registered callback (`BleCommandCallback`) is invoked for each complete line.

## Filesystem Layout

```
data/lyrics/
  ├── Tumi.txt      Timestamped lyrics (format: <mm:ss.ff>word)
  └── Closer.txt    Timestamped lyrics
```

Lyrics format: `[Section] <00:05.30>word1 <00:05.80>word2 ...` parsed by `LyricsApp::ParseLine()`.

## Build System

- **PlatformIO** with `espressif32` platform, `esp32doit-devkit-v1` board.
- **Filesystem:** LittleFS (upload with `pio run -t uploadfs`).
- **Dependency:** `olikraus/U8g2` library.
- **Build scripts:** `build_pio.bat`, `scripts/do_build.ps1`.

## Adding New Components

### Adding a new app

1. Add an `AppId` value in `src/common/event_types.h` before `Count`.
2. Create `src/apps/<name>/<name>_app.h` and `.cpp` implementing `IApp`.
3. Define a global instance (e.g., `extern MyApp my_app;` in header, `MyApp my_app;` in cpp).
4. Register in `main.cpp`: `app_manager.RegisterApp(&my_app);` and call `my_app.Begin()`.
5. Subscribe to `DisplayRequest` events if the app needs to draw (or post directly — DisplayService already subscribed).

### Adding a new service

1. Create the service class implementing `IEventSubscriber` if it reacts to events.
2. Define a global instance.
3. Wire into `EventBus` subscription in `main.cpp` if needed.
