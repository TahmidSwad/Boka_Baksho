# Current State

## Completed

- **Core framework:** EventBus, AppManager (registry + nav stack), IApp interface — fully implemented and tested via serial.
- **Drivers:** Oled (U8g2 SSD1306, I2C probe with fallback), Button (debounced active-low) — working.
- **Services:** DisplayService (foreground-only rendering), InputService (4 buttons + serial debug), StorageService (LittleFS), BleService (GATT text protocol) — implemented and wired.
- **AudioService:** Stub only (no implementation).
- **InputService:** Hardware increment/decrement buttons now map to `ButtonIncrement`/`ButtonDecrement` (not RotateRight/RotateLeft). RotateLeft/RotateRight remain available via serial debug (`l`/`r`).
- **SystemUI launcher:** Scrollable menu with wraparound navigation using ButtonIncrement/ButtonDecrement (hardware) or RotateLeft/RotateRight (serial). ENTER to launch, BACK ignored at root.
- **LyricsApp:** File list navigated with increment/decrement buttons, selected song shown with `>` cursor. ENTER selects, BLE sends `PLAY|song_name` to PC, waits for `AUDIO_STARTED` handshake (10s timeout), then starts timed lyric display. Pause/resume with ENTER, BACK returns to file list. All error states (load failed, handshake timeout) are non-blocking with 1.5s auto-return or immediate return on any button press.
- **StopwatchApp:** Dual-mode stopwatch (count up) and timer (count down). Encoder sets minutes, ENTER starts/pauses, BACK resets. Blinking "TIME'S UP!" notification.
- **BLE protocol:** GATT service with TX (notify) / RX (write), text commands, connection/disconnection handling, advertising restart on disconnect.
- **Lyric files:** `data/lyrics/Tumi.txt`, `data/lyrics/Closer.txt` — timestamped lyric files uploaded via LittleFS.

## In Progress / Partially Implemented

- **Audio playback:** `AudioService` is a stub. No actual audio hardware control exists.
- **Music app:** `AppId::Music` defined but not implemented. No class, no registration.
- **Alarm app:** `AppId::Alarm` defined but not implemented.
- **Settings app:** `AppId::Settings` defined but not implemented.

## Known Issues / Limitations

- **BLE handshake timeout:** LyricsApp waits 10s for `AUDIO_STARTED`. If the Python client is slow to connect, the user sees "PC Timeout" and returns to the file list. This is working as designed but could be improved with a retry option.
- **No audio output:** The device cannot play audio. The lyrics app relies entirely on a connected Python client for audio.
- **Single BLE client:** Only one BLE connection at a time. Advertising restarts on disconnect.
- **Lyrics file count cap:** Maximum 16 lyric files (`kMaxLyricFiles`). Files beyond this are silently ignored.
- **Word display timeout:** Each word clears after 5 seconds (`kWordDisplayTimeoutMs`). Long silences between words may cause the display to go blank.
- **Stopwatch precision:** Uses `millis()` delta-based timing. Drift accumulates over long periods (no NTP or RTC).
- **No OTA updates:** Firmware updates require physical USB connection.
- **Use-after-free risk in EventBus event pointers:** Events that store pointers to caller-local data (e.g., stack arrays) become dangling after the caller returns. This was the root cause of the LyricsApp black-screen bug (fixed 2026-09-15). Other apps (SystemUi, StopwatchApp) use member variables for their event payloads and are not affected.
- **"Load failed" in LyricsApp:** If this appears, the lyrics files may not be on the device. Run `pio run -t uploadfs` to upload `data/lyrics/` to LittleFS. Check serial output for `LYRICS: open failed` or `LYRICS: WARNING - no timestamped words found` to diagnose.

## Pending Tasks

- Implement `AudioService` for actual audio playback (hardware TBD).
- Implement `Music`, `Alarm`, and `Settings` apps as needed.
- Consider adding a word highlighting/scroll mechanism for lyrics (currently single word centered).
- Consider adding a progress indicator for lyrics playback position.

## TODOs in Code

- `ble_service.cpp:146`: `(void)event;` — System event posting from BLE received data is placeholder, kept for future expansion.
- `main.cpp:81-93`: BLE command handler has stub branches for `STOPPED`, `PAUSED`, `RESUMED`, `TIME_ACK` — lyrics app handles these internally but the callback exists for future use.
