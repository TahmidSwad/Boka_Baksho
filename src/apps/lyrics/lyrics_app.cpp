#include "apps/lyrics/lyrics_app.h"

#include "core/system.h"
#include "services/storage_service.h"
#include "services/ble_service.h"
#include "common/event_types.h"
#include "common/config.h"

LyricsApp lyrics_app;

// ==========================================================
// LIFECYCLE
// ==========================================================

bool LyricsApp::Begin() {
  ClearData();
  ScanLyricFiles();
  return true;
}

bool LyricsApp::OnActivate() {
  EnterFileList();
  Serial.println("LYRICS_ON");
  return true;
}

void LyricsApp::OnDeactivate() {
  StopPlayback();
  ClearDisplay();
  Serial.println("LYRICS_OFF");
}

// ==========================================================
// BLE HANDLER
// ==========================================================

void LyricsApp::OnAudioStarted() {
  if (state_ == State::WaitingHandshake) {
    Serial.println("BLE: AUDIO_STARTED received");
    audio_started_ = true;
  }
}

// ==========================================================
// INPUT HANDLING
// ==========================================================

bool LyricsApp::HandleInput(const InputEvent& event) {
  switch (state_) {
    case State::FileList:
      return HandleFileListInput(event);
    case State::Loading:
    case State::WaitingHandshake:
      if (event.type == InputType::Back) {
        ReturnToFileList();
        return true;
      }
      return false;
    case State::LoadFailed:
      ReturnToFileList();
      return true;
    case State::Playing:
    case State::Paused:
      return HandlePlaybackInput(event);
    default:
      return false;
  }
}

bool LyricsApp::HandleFileListInput(const InputEvent& event) {
  if (lyric_file_count_ == 0) {
    if (event.type == InputType::Back) return false;
    return true;
  }
  switch (event.type) {
    case InputType::ButtonDecrement:
    case InputType::RotateLeft:
      if (selected_file_index_ > 0) {
        --selected_file_index_;
        if (selected_file_index_ < list_scroll_) {
          --list_scroll_;
        }
      }
      ShowFileList();
      return true;
    case InputType::ButtonIncrement:
    case InputType::RotateRight:
      if (selected_file_index_ < lyric_file_count_ - 1) {
        ++selected_file_index_;
        if (selected_file_index_ >= list_scroll_ + kVisibleLines) {
          if (list_scroll_ + kVisibleLines < lyric_file_count_) {
            ++list_scroll_;
          }
        }
      }
      ShowFileList();
      return true;
    case InputType::Enter:
      LoadSelectedLyric();
      return true;
    case InputType::Back:
      return false;
  }
  return false;
}

bool LyricsApp::HandlePlaybackInput(const InputEvent& event) {
  switch (event.type) {
    case InputType::Enter:
      if (state_ == State::Playing) PausePlayback();
      else if (state_ == State::Paused) ResumePlayback();
      return true;
    case InputType::Back:
      ReturnToFileList();
      return true;
  }
  return false;
}

// ==========================================================
// FILE LISTING
// ==========================================================

void LyricsApp::ScanLyricFiles() {
  lyric_file_count_ = 0;
  if (!storage_service.IsReady()) return;
  const char* dir_path = "/lyrics";
  File dir = storage_service.Open(dir_path, FILE_READ);
  if (!dir || !dir.isDirectory()) {
    Serial.println("LYRICS: /lyrics dir not found");
    return;
  }
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;
    if (!entry.isDirectory()) {
      const char* name = entry.name();
      const char* base = strrchr(name, '/');
      base = base ? base + 1 : name;
      size_t base_len = strlen(base);
      if (base_len > 4 && strcmp(base + base_len - 4, ".txt") == 0) {
        if (lyric_file_count_ < kMaxLyricFiles) {
          LyricFileInfo& info = lyric_files_[lyric_file_count_];
          snprintf(info.filename, sizeof(info.filename), "%s/%s", dir_path, base);
          if (base_len > 4) {
            snprintf(info.display_name, sizeof(info.display_name), "%.*s",
                     (int)(base_len - 4), base);
          } else {
            snprintf(info.display_name, sizeof(info.display_name), "%s", base);
          }
          Serial.print("LYRICS: found ");
          Serial.println(info.filename);
          ++lyric_file_count_;
        }
      }
    }
    entry.close();
  }
  dir.close();
  Serial.print("LYRICS: total ");
  Serial.println(lyric_file_count_);
}

bool LyricsApp::LoadLyrics(const char* filename) {
  if (filename == nullptr) return false;
  ClearData();
  File file = storage_service.Open(filename, FILE_READ);
  if (!file) {
    Serial.print("LYRICS: open failed: ");
    Serial.println(filename);
    return false;
  }
  Serial.print("LYRICS: parsing ");
  Serial.println(filename);
  char linebuf[512];
  while (file.available()) {
    char c = file.read();
    if (c == '\n' || c == '\r') {
      if (linebuf[0] != '\0') {
        linebuf[511] = '\0';
        ParseLine(linebuf);
        linebuf[0] = '\0';
      }
    } else if (c > 0) {
      size_t len = strlen(linebuf);
      if (len < 511) {
        linebuf[len] = (char)c;
        linebuf[len + 1] = '\0';
      }
    }
  }
  if (linebuf[0] != '\0') {
    linebuf[511] = '\0';
    ParseLine(linebuf);
  }
  file.close();
  Serial.print("LYRICS: parsed ");
  Serial.print(word_count_);
  Serial.println(" words");
  if (word_count_ == 0) {
    Serial.println("LYRICS: WARNING - no timestamped words found");
  }
  return word_count_ > 0;
}

void LyricsApp::LoadSelectedLyric() {
  if (selected_file_index_ >= lyric_file_count_) return;
  const char* filename = lyric_files_[selected_file_index_].filename;
  if (!LoadLyrics(filename)) {
    ShowStatus("Load failed", TextSize::Medium);
    Serial.print("LYRICS: load failed for: ");
    Serial.println(filename);
    state_ = State::LoadFailed;
    load_failed_start_ = millis();
    return;
  }
  EnterLoading();
  SendPlayCommand(lyric_files_[selected_file_index_].display_name);
  EnterWaitingHandshake();
}


// ==========================================================
// STATE TRANSITIONS
// ==========================================================

void LyricsApp::EnterFileList() {
  state_ = State::FileList;
  loaded_ = false;
  playing_ = false;
  paused_ = false;
  new_word_ = false;
  display_active_ = false;
  selected_file_index_ = 0;
  list_scroll_ = 0;
  ShowFileList();
}

void LyricsApp::EnterLoading() {
  state_ = State::Loading;
  ShowStatus("Loading...", TextSize::Medium);
}

void LyricsApp::EnterWaitingHandshake() {
  state_ = State::WaitingHandshake;
  handshake_start_ = millis();
  ShowStatus("Wait PC", TextSize::Medium);
}

void LyricsApp::EnterPlaying() {
  state_ = State::Playing;
  playing_ = true;
  paused_ = false;
  loaded_ = true;
  start_time_ = millis();
  current_word_ = 0;
  last_shown_word_ = UINT16_MAX;
  new_word_ = false;
  display_active_ = false;
  title_until_ = millis() + kTitleTimeoutMs;
}

void LyricsApp::EnterPaused() {
  state_ = State::Paused;
  paused_ = true;
  playing_ = false;
  pause_time_ = millis();
  ShowStatus("PAUSED", TextSize::Medium);
}

void LyricsApp::ReturnToFileList() {
  StopPlayback();
  EnterFileList();
}

// ==========================================================
// PLAYBACK CONTROL
// ==========================================================

void LyricsApp::StartPlayback() {
  playing_ = true;
  paused_ = false;
  start_time_ = millis();
  current_word_ = 0;
  new_word_ = false;
  display_active_ = false;
  title_until_ = millis() + kTitleTimeoutMs;
  state_ = State::Playing;
}

void LyricsApp::PausePlayback() {
  if (state_ != State::Playing) return;
  EnterPaused();
}

void LyricsApp::ResumePlayback() {
  if (state_ != State::Paused) return;
  start_time_ += millis() - pause_time_;
  state_ = State::Playing;
  playing_ = true;
  paused_ = false;
  new_word_ = true;
}

void LyricsApp::StopPlayback() {
  playing_ = false;
  paused_ = false;
  loaded_ = false;
  new_word_ = false;
  display_active_ = false;
  ClearDisplay();
}

// ==========================================================
// DISPLAY
// ==========================================================

void LyricsApp::ShowFileList() {
  if (lyric_file_count_ == 0) {
    ShowStatus("No lyrics", TextSize::Medium);
    return;
  }
  uint8_t visible = 0;
  for (uint8_t i = 0; i < kVisibleLines; ++i) {
    uint8_t idx = list_scroll_ + i;
    if (idx >= lyric_file_count_) break;
    visible_lines_[i] = lyric_files_[idx].display_name;
    ++visible;
  }
  Event event;
  event.type = EventType::DisplayRequest;
  event.sender = GetAppId();
  event.display_request.type = DisplayRequestType::ShowLines;
  event.display_request.lines = visible_lines_;
  event.display_request.line_count = visible;
  event.display_request.selected = selected_file_index_ - list_scroll_;
  event.display_request.text_size = TextSize::Small;
  event.display_request.alignment = TextAlign::Left;
  display_active_ = true;
  (void)event_bus.Post(event);
}

void LyricsApp::ShowStatus(const char* text, TextSize size) {
  Event event;
  event.type = EventType::DisplayRequest;
  event.sender = GetAppId();
  event.display_request.type = DisplayRequestType::ShowWord;
  event.display_request.text = text;
  event.display_request.text_size = size;
  event.display_request.alignment = TextAlign::Center;
  display_active_ = true;
  (void)event_bus.Post(event);
}

bool LyricsApp::ShowCurrentWord() {
  const char* word = GetCurrentWord();
  if (word == nullptr) return false;
  Event event;
  event.type = EventType::DisplayRequest;
  event.sender = GetAppId();
  event.display_request.type = DisplayRequestType::ShowWord;
  event.display_request.text = word;
  event.display_request.text_size = TextSize::Medium;
  event.display_request.alignment = TextAlign::Center;
  display_active_ = true;
  return (bool)event_bus.Post(event);
}

bool LyricsApp::ClearDisplay() {
  Event event;
  event.type = EventType::DisplayRequest;
  event.sender = GetAppId();
  event.display_request.type = DisplayRequestType::ClearDisplay;
  display_active_ = false;
  return event_bus.Post(event);
}

// ==========================================================
// BLE
// ==========================================================

void LyricsApp::SendPlayCommand(const char* song_name) {
  if (song_name == nullptr) return;
  char cmd[128];
  snprintf(cmd, sizeof(cmd), "PLAY|%s", song_name);
  if (ble_service.IsConnected()) {
    ble_service.SendCommand(cmd);
    Serial.print("BLE TX: ");
    Serial.println(cmd);
  } else {
    Serial.println("BLE: not connected");
  }
}


// ==========================================================
// HELPERS
// ==========================================================

void LyricsApp::ClearData() {
  word_count_ = 0;
  current_word_ = 0;
  last_shown_word_ = UINT16_MAX;
  start_time_ = 0;
  pause_time_ = 0;
  title_until_ = 0;
  handshake_start_ = 0;
  loaded_ = false;
  playing_ = false;
  paused_ = false;
  new_word_ = false;
  display_active_ = false;
}

const char* LyricsApp::GetCurrentWord() const {
  if (!loaded_ || current_word_ >= word_count_) return nullptr;
  return words_[current_word_].word;
}

// ==========================================================
// UPDATE
// ==========================================================

void LyricsApp::Update() {
  switch (state_) {
    case State::WaitingHandshake: {
      if (audio_started_) {
        audio_started_ = false;
        ClearDisplay();
        EnterPlaying();
        break;
      }
      if (millis() - handshake_start_ > kHandshakeTimeoutMs) {
        Serial.println("BLE: Handshake timeout");
        ShowStatus("PC Timeout", TextSize::Medium);
        state_ = State::LoadFailed;
        load_failed_start_ = millis();
      }
      break;
    }
    case State::LoadFailed: {
      if (millis() - load_failed_start_ > kLoadFailedTimeoutMs) {
        ReturnToFileList();
      }
      break;
    }
    case State::Playing: {
      if (!playing_) return;
      if (millis() < title_until_) return;
      uint32_t elapsed = millis() - start_time_;
      UpdateCurrentWord(elapsed);
      if (new_word_) {
        if (ShowCurrentWord()) {
          new_word_ = false;
          last_shown_word_ = current_word_;
          word_display_start_ = millis();
        }
        return;
      }
      if (!display_active_) return;
      if (millis() - word_display_start_ < kWordDisplayTimeoutMs) return;
      ClearDisplay();
      break;
    }
    default:
      break;
  }
}

// ==========================================================
// PARSING
// ==========================================================

bool LyricsApp::ParseLine(const char* line) {
  if (line == nullptr) return false;
  const char* p = line;
  if (p[0] == '[') {
    while (*p && *p != ']') p++;
    if (*p == ']') p++;
    while (*p == ' ') p++;
  }
  bool added_any = false;
  while (*p && word_count_ < kMaxWords) {
    if (*p == '<') {
      uint32_t ms = 0;
      uint8_t consumed = 0;
      if (!ParseWordTimestamp(p, ms, consumed)) { p++; continue; }
      const char* text = p + consumed;
      uint8_t i = 0;
      while (i < kMaxWordLength - 1 && text[i] != '\0' && text[i] != ' '
             && text[i] != '\n' && text[i] != '\r') {
        words_[word_count_].word[i] = text[i];
        ++i;
      }
      words_[word_count_].word[i] = '\0';
      words_[word_count_].timestamp = ms;
      ++word_count_;
      added_any = true;
      p += consumed;
      while (*p == ' ') p++;
      continue;
    }
    p++;
  }
  return added_any;
}
bool LyricsApp::ParseWordTimestamp(const char* text, uint32_t& milliseconds, uint8_t& consumed) {
  if (text == nullptr || strlen(text) < 10) return false;
  if (text[0] != '<' || text[3] != ':' || text[6] != '.' || text[9] != '>') return false;
  for (uint8_t i = 1; i <= 2; ++i) if (!isdigit(text[i])) return false;
  for (uint8_t i = 4; i <= 5; ++i) if (!isdigit(text[i])) return false;
  for (uint8_t i = 7; i <= 8; ++i) if (!isdigit(text[i])) return false;
  uint32_t minutes = (text[1] - '0') * 10 + (text[2] - '0');
  uint32_t seconds = (text[4] - '0') * 10 + (text[5] - '0');
  uint32_t hundredths = (text[7] - '0') * 10 + (text[8] - '0');
  if (seconds >= 60) return false;
  milliseconds = (minutes * 60UL * 1000UL) + (seconds * 1000UL) + (hundredths * 10UL);
  consumed = 10;
  return true;
}

void LyricsApp::UpdateCurrentWord(uint32_t elapsed) {
  while (current_word_ + 1 < word_count_ &&
         words_[current_word_ + 1].timestamp <= elapsed) {
    ++current_word_;
    new_word_ = true;
  }
  if (!new_word_ && !display_active_ && current_word_ < word_count_ &&
      elapsed >= words_[current_word_].timestamp &&
      current_word_ != last_shown_word_) {
    new_word_ = true;
    last_shown_word_ = current_word_;
  }
}
