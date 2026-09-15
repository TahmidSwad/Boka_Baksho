# Change Log

## 2026-09-15 — Lyrics UI: selection indicator + non-blocking error states

- **DisplayService::ShowLines** now renders a `>` cursor before the selected line when `req.selected < req.line_count`. Lines are left-aligned with 10px indent for the cursor.
- **LyricsApp::ShowFileList** passes `selected = selected_file_index_ - list_scroll_` so the current song is visually marked.
- **LoadFailed state** added: replaces blocking `delay(1000)` in `LoadSelectedLyric` and `Update` with a non-blocking timer (`kLoadFailedTimeoutMs = 1500ms`). Any button press during LoadFailed returns to file list immediately.
- **Handshake timeout** also made non-blocking (was `delay(1000)`, now uses `LoadFailed` state).
- **LoadLyrics** improved serial logging: prints filename being parsed, word count, and a warning when zero words are found.

## 2026-09-15 — Refactor input: ButtonIncrement/ButtonDecrement as primary navigation

- **InputService** now maps hardware increment button -> `ButtonIncrement`, decrement -> `ButtonDecrement` (was RotateRight/RotateLeft). Serial debug: `+`/`-` still post ButtonIncrement/ButtonDecrement; `l`/`r` post RotateLeft/RotateRight.
- **SystemUi** handles both `ButtonIncrement`/`ButtonDecrement` and `RotateLeft`/`RotateRight` for launcher navigation.
- **LyricsApp** handles both `ButtonIncrement`/`ButtonDecrement` and `RotateLeft`/`RotateRight` for file list navigation.
- **Bug fix:** `HandlePlaybackInput` and Loading/WaitingHandshake states now return `false` for unconsumed events instead of `true`, allowing the AppManager to handle them (e.g., Back fallback to launcher).

## 2026-09-15 — Fix dangling pointer in LyricsApp::ShowFileList()

- **Bug:** `ShowFileList()` allocated a local `const char* lines[6]` on the stack and stored its address in a `DisplayRequest` event. When `ShowFileList()` returned, the stack memory was reclaimed but the queued event still held the pointer. `DisplayService::ShowLines()` then dereferenced this dangling pointer, causing undefined behavior (crash/reboot, black screen, or garbage display).
- **Fix:** Promoted `lines` to a member variable `visible_lines_[kVisibleLines]` of `LyricsApp`. The pointer remains valid for the lifetime of the app.
- **Files changed:** `src/apps/lyrics/lyrics_app.h` (added member), `src/apps/lyrics/lyrics_app.cpp` (use member instead of local).

## 2026-09-15 — Project Context Established

- Created `AGENTS.md` with project overview, conventions, constraints, and workflow rules.
- Created `docs/architecture.md` with layered architecture, data flows, navigation stack, app lifecycle, BLE protocol, and filesystem layout.
- Created `docs/decisions.md` with 10 architectural decisions (static allocation, nav stack ownership, event bus, SystemUi root, IApp interface, BLE text protocol, InputService, OLED probing, lyrics format, no ResourceManager).
- Created `docs/current-state.md` with completed features, partial implementations, known issues, and pending tasks.
- Created `docs/change-log.md` (this file).
