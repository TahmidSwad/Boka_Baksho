# Architectural Decisions

## D1: Static allocation only (no dynamic memory)

**Chosen:** All data structures use compile-time-sized static arrays. No `new`, `malloc`, `std::vector`, or heap allocation.

**Why:** ESP32 has limited RAM and no memory protection. Dynamic allocation risks fragmentation and unpredictable failures. Static allocation makes memory usage deterministic and eliminates leaks.

**Rejected:** `std::vector`, `String` (Arduino), `new`/`delete`. The codebase uses only C-strings (`char[]`, `const char*`) and fixed arrays.

---

## D2: Navigation stack as the ownership mechanism

**Chosen:** Display ownership is implicit via the navigation stack. Only the app at the top of the stack (the foreground app) can draw. Enforced by `DisplayService` checking `event.sender == app_manager.GetForeground()->GetAppId()`.

**Why:** Simple, zero overhead. No separate resource manager needed. The nav stack already tracks which app is visible.

**Rejected:** Explicit `ResourceManager` class with acquire/release semantics. Overkill for a single shared display.

---

## D3: Event bus with typed subscriber lists

**Chosen:** A single `EventBus` with a ring-buffer queue and per-`EventType` subscriber arrays (max 4 subscribers per type, 16 events in queue).

**Why:** Decouples publishers from consumers. Apps post events without knowing who handles them. Services subscribe to specific event types. The ring buffer prevents unbounded growth.

**Rejected:** Direct function calls between components (tight coupling), observer pattern with dynamic registration (requires allocation).

---

## D4: SystemUi as the nav stack root

**Chosen:** The launcher (`SystemUi`) is always at the bottom of the navigation stack. It can never be popped. Launching an app pushes on top; Back pops back to it.

**Why:** Guarantees there is always a foreground app. Simplifies the Back-button logic (unconsumed Back in any app returns to the launcher). The launcher is the "home screen."

**Rejected:** Making the launcher a special case outside the stack. The unified stack model is simpler and consistent.

---

## D5: IApp interface for all applications

**Chosen:** All apps implement the `IApp` interface with lifecycle methods: `Begin()`, `OnActivate()`, `OnDeactivate()`, `Update()`, `HandleInput()`.

**Why:** Uniform treatment by `AppManager`. Apps can be pushed/popped without the manager knowing implementation details. The lifecycle methods provide clear hooks for setup, teardown, and per-frame updates.

**Rejected:** Inheriting from a base class with default behavior. The interface is intentionally minimal — apps override only what they need.

---

## D6: BLE as a text-based protocol

**Chosen:** Plain newline-terminated text commands over BLE GATT (TX notify / RX write). Commands like `PLAY|song_name`, `AUDIO_STARTED`, `TIME_SYNC|<ms>`.

**Why:** Simple to parse, debug (visible in serial monitor), and generate from a Python client. No binary serialization complexity.

**Rejected:** Binary protocol with length-prefixed frames. Unnecessary complexity for the small number of commands.

---

## D7: Separate InputService for button polling

**Chosen:** `InputService` owns all four `Button` objects, polls them every loop iteration, and posts normalized `InputEvent` structs to the `EventBus`. Also provides serial debug input (`l/r/e/b/+/-`).

**Why:** Centralizes hardware input handling. Apps receive abstract `InputEvent`s, not raw pin reads. Serial debug enables testing without hardware.

**Rejected:** Each app polling its own buttons (scattered hardware access, no normalization).

---

## D8: OLED I2C address probing

**Chosen:** At boot, `Oled::Begin()` probes `0x3C` first, then `0x3D` (common clone fallback). Logs the result to serial. Returns `false` if no device found.

**Why:** Some SSD1306 clones use `0x3D` instead of `0x3C`. Probing at boot avoids runtime failures. The `DisplayService::Begin()` propagates the failure so the system can halt gracefully.

**Rejected:** Hardcoding a single address and hoping it works. The probe adds ~2ms at boot with no runtime cost.

---

## D9: Lyrics timestamp format

**Chosen:** `<mm:ss.ff>word` format (e.g., `<01:23.45>hello`). Parsed by `LyricsApp::ParseWordTimestamp()`.

**Why:** Human-readable, easy to author manually. The `ff` hundredths-of-a-second precision is sufficient for karaoke sync.

**Rejected:** Millisecond-only timestamps (`<83450>hello`). Less readable. Frame-based timestamps. Unnecessary complexity.

---

## D10: No ResourceManager — display exclusivity via nav stack

**Chosen:** The `DisplayService` checks `event.sender == app_manager.GetForeground()->GetAppId()` before rendering. No separate resource tracking.

**Why:** For a single shared display, the nav stack already tracks the foreground app. Adding a ResourceManager would duplicate this information.

**Rejected:** Explicit `ResourceManager` with lock/unlock. Would add a new subsystem and API surface for no benefit.
