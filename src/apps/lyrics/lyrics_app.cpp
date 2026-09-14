#include "apps/lyrics/lyrics_app.h"

#include "core/system.h"
#include "services/storage_service.h"
#include "common/event_types.h"
#include "common/config.h"

LyricsApp lyrics_app;

namespace {
const char* kLyricFiles[] = {config::kLyricsPathA, config::kLyricsPathB};
const char* kLyricNames[] = {"Tumi", "Closer"};
}  // namespace

// ==========================================================
// LIFECYCLE
// ==========================================================

bool LyricsApp::Begin() {
  ClearData();
  file_index_ = 0;
  return true;
}

bool LyricsApp::OnActivate() {
  if (!LoadLyrics(kLyricFiles[file_index_])) {
    Serial.println("LYRICS_LOAD_FAILED");
    return false;
  }
  Start();
  Serial.println("LYRICS_ON");
  return true;
}

void LyricsApp::OnDeactivate() {
  playing_ = false;
  paused_ = false;
  start_time_ = 0;
  pause_time_ = 0;
  current_word_ = 0;
  word_display_start_ = 0;
  title_until_ = 0;
  new_word_ = false;
  display_active_ = false;
  ClearDisplay();
  Serial.println("LYRICS_OFF");
}

// ==========================================================
// INPUT
// ==========================================================
//   encoder left/right -> switch lyric file
//   ENTER              -> play / pause toggle
//   BACK               -> not consumed: system exits to launcher
// ==========================================================

bool LyricsApp::HandleInput(const InputEvent& event) {
  switch (event.type) {
    case InputType::RotateLeft:
      SwitchFile(-1);
      return true;
    case InputType::RotateRight:
      SwitchFile(1);
      return true;
    case InputType::Enter:
      TogglePlayback();
      return true;
    case InputType::Back:
      return false;
  }
  return true;
}

// ==========================================================
// UPDATE
// ==========================================================

void LyricsApp::Update() {
  if (!loaded_ || !playing_) return;

  // Keep the status/title visible during the intro window.
  if (millis() < title_until_) return;

  uint32_t elapsed = millis() - start_time_;
  UpdateCurrentWord(elapsed);

  if (new_word_) {
    if (ShowCurrentWord()) new_word_ = false;
    return;
  }

  if (!display_active_) return;
  if (millis() - word_display_start_ < kWordDisplayTimeoutMs) return;

  ClearDisplay();
}

// ==========================================================
// LOAD LYRICS
// ==========================================================

bool LyricsApp::LoadLyrics(const char* filename) {
  if (filename == nullptr || !storage_service.IsReady()) return false;

  File file = storage_service.Open(filename, FILE_READ);
  if (!file) return false;

  ClearData();

  char line[512];
  while (file.available()) {
    size_t length = file.readBytesUntil('\n', line, sizeof(line) - 1);
    line[length] = '\0';
    if (length > 0 && line[length - 1] == '\r') {
      line[length - 1] = '\0';
    }
    if (!ParseLine(line)) {
      file.close();
      ClearData();
      return false;
    }
  }

  file.close();

  if (word_count_ == 0) {
    ClearData();
    return false;
  }

  loaded_ = true;
  current_word_ = 0;
  start_time_ = 0;
  pause_time_ = 0;
  word_display_start_ = 0;
  title_until_ = 0;
  playing_ = false;
  paused_ = false;
  new_word_ = false;
  display_active_ = false;
  return true;
}

// ==========================================================
// PLAYBACK CONTROL
// ==========================================================

void LyricsApp::Start() {
  if (!loaded_) return;
  current_word_ = 0;
  start_time_ = millis();
  pause_time_ = 0;
  word_display_start_ = 0;
  title_until_ = millis() + kTitleTimeoutMs;
  playing_ = true;
  paused_ = false;
  new_word_ = false;
  display_active_ = false;
  ShowStatus(kLyricNames[file_index_], TextSize::Large);
}

void LyricsApp::TogglePlayback() {
  if (!loaded_) return;

  if (playing_) {
    // Pause.
    playing_ = false;
    paused_ = true;
    pause_time_ = millis();
    new_word_ = false;
    word_display_start_ = 0;
    display_active_ = false;
    ShowStatus("PAUSED", TextSize::Medium);
    return;
  }

  if (paused_) {
    // Resume, keeping the timeline position.
    uint32_t now = millis();
    start_time_ += now - pause_time_;
    pause_time_ = 0;
    paused_ = false;
    playing_ = true;
    new_word_ = false;
    word_display_start_ = 0;
    title_until_ = millis() + kTitleTimeoutMs;
    ShowStatus(kLyricNames[file_index_], TextSize::Large);
  }
}

void LyricsApp::SwitchFile(int8_t dir) {
  file_index_ = (uint8_t)((file_index_ + dir + 2) % 2);
  if (!LoadLyrics(kLyricFiles[file_index_])) {
    Serial.println("LYRICS_LOAD_FAILED");
    return;
  }
  Start();
  Serial.println("LYRICS_FILE_SWITCHED");
}

// ==========================================================
// DISPLAY HELPERS
// ==========================================================

bool LyricsApp::ShowCurrentWord() {
  const char* word = GetCurrentWord();
  if (word == nullptr) return false;

  Event event;
  event.type = EventType::DisplayRequest;
  event.sender = GetAppId();
  event.display_request.type = DisplayRequestType::ShowWord;
  event.display_request.text = word;
  event.display_request.text_size = TextSize::Large;
  event.display_request.alignment = TextAlign::Center;

  if (!event_bus.Post(event)) return false;

  word_display_start_ = millis();
  display_active_ = true;
  return true;
}

bool LyricsApp::ShowStatus(const char* text, TextSize size) {
  if (text == nullptr) return false;

  Event event;
  event.type = EventType::DisplayRequest;
  event.sender = GetAppId();
  event.display_request.type = DisplayRequestType::ShowLine;
  event.display_request.text = text;
  event.display_request.text_size = size;
  event.display_request.alignment = TextAlign::Center;

  return event_bus.Post(event);
}

bool LyricsApp::ClearDisplay() {
  Event event;
  event.type = EventType::DisplayRequest;
  event.sender = GetAppId();
  event.display_request.type = DisplayRequestType::ClearDisplay;

  if (!event_bus.Post(event)) return false;

  display_active_ = false;
  word_display_start_ = 0;
  return true;
}

const char* LyricsApp::GetCurrentWord() const {
  if (!loaded_ || current_word_ >= word_count_) return nullptr;
  return words_[current_word_].word;
}

void LyricsApp::ClearData() {
  word_count_ = 0;
  current_word_ = 0;
  start_time_ = 0;
  pause_time_ = 0;
  word_display_start_ = 0;
  title_until_ = 0;
  loaded_ = false;
  playing_ = false;
  paused_ = false;
  new_word_ = false;
  display_active_ = false;
}

// ==========================================================
// TIMELINE
// ==========================================================

void LyricsApp::UpdateCurrentWord(uint32_t elapsed) {
  if (current_word_ == 0 && !display_active_ && !new_word_ &&
      elapsed >= words_[0].timestamp) {
    new_word_ = true;
    return;
  }

  while (current_word_ + 1 < word_count_ &&
         words_[current_word_ + 1].timestamp <= elapsed) {
    ++current_word_;
    new_word_ = true;
  }
}

// ==========================================================
// PARSING
// ==========================================================

bool LyricsApp::ParseLine(const char* line) {
  if (line == nullptr) return false;
  const char* cursor = line;

  if (*cursor == '[') {
    const char* closing = strchr(cursor, ']');
    if (closing != nullptr) {
      cursor = closing + 1;
    }
  }

  while (*cursor != '\0') {
    const char* ts_start = strchr(cursor, '<');
    if (ts_start == nullptr) break;

    uint32_t timestamp = 0;
    uint8_t consumed = 0;
    if (!ParseWordTimestamp(ts_start, timestamp, consumed)) {
      return false;
    }

    const char* word_start = ts_start + consumed;
    const char* next_ts = strchr(word_start, '<');
    const char* word_end = (next_ts != nullptr) ? next_ts
                                                : word_start + strlen(word_start);

    while (word_start < word_end && (*word_start == ' ' || *word_start == '\t')) {
      ++word_start;
    }
    while (word_end > word_start && (word_end[-1] == ' ' || word_end[-1] == '\t')) {
      --word_end;
    }

    size_t word_len = word_end - word_start;
    if (word_len > 0) {
      if (word_count_ >= kMaxWords) return false;
      if (word_len >= kMaxWordLength) return false;

      memcpy(words_[word_count_].word, word_start, word_len);
      words_[word_count_].word[word_len] = '\0';
      words_[word_count_].timestamp = timestamp;
      ++word_count_;
    }

    if (next_ts == nullptr) break;
    cursor = next_ts;
  }

  return true;
}

bool LyricsApp::ParseWordTimestamp(const char* text, uint32_t& milliseconds,
                                   uint8_t& consumed) {
  if (text == nullptr || strlen(text) < 10) return false;
  if (text[0] != '<' || text[3] != ':' || text[6] != '.' || text[9] != '>') {
    return false;
  }

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