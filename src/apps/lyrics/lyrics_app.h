#ifndef LYRICS_APP_H
#define LYRICS_APP_H

#include <Arduino.h>

#include "core/app_base.h"

// ==========================================================
// LYRICS APP
// ==========================================================
// 1. Shows a scrollable list of all lyric files on LittleFS.
// 2. User selects one with ENTER.
// 3. Lyrics are loaded and parsed (timestamped words).
// 4. PLAY|filename is sent to the PC over BLE.
// 5. App waits for AUDIO_STARTED handshake (10s timeout).
// 6. Once received, lyric timing starts from zero.
// ==========================================================

class LyricsApp : public IApp {
public:
  bool Begin() override;
  bool OnActivate() override;
  void OnDeactivate() override;
  void Update() override;
  bool HandleInput(const InputEvent& event) override;
  AppId GetAppId() const override { return AppId::Lyrics; }
  const char* GetName() const override { return "Lyrics"; }

  // Called by BLE command handler when PC sends AUDIO_STARTED.
  void OnAudioStarted();

private:
  // ---- App States ----
  enum class State {
    FileList,
    Loading,
    WaitingHandshake,
    LoadFailed,
    Playing,
    Paused
  };

  // ---- Constants ----
  static constexpr uint16_t kMaxWords = 1000;
  static constexpr uint8_t  kMaxWordLength = 32;
  static constexpr uint32_t kTitleTimeoutMs = 1500;
  static constexpr uint32_t kWordDisplayTimeoutMs = 5000;
  static constexpr uint8_t  kMaxLyricFiles = 16;
  static constexpr uint32_t kHandshakeTimeoutMs = 10000;
  static constexpr uint32_t kLoadFailedTimeoutMs = 1500;
  static constexpr uint8_t  kVisibleLines = 6;

  // ---- Lyric Word ----
  struct LyricWord {
    uint32_t timestamp;
    char     word[kMaxWordLength];
  };

  // ---- Lyric File Info ----
  struct LyricFileInfo {
    char filename[64];    // e.g. "/lyrics/Tumi.txt"
    char display_name[32]; // e.g. "Tumi"
  };

  // ---- Parsed Lyric Data ----
  LyricWord words_[kMaxWords];
  uint16_t word_count_ = 0;
  uint16_t current_word_ = 0;
  uint16_t last_shown_word_ = UINT16_MAX;

  // ---- Timing ----
  uint32_t start_time_ = 0;
  uint32_t pause_time_ = 0;
  uint32_t word_display_start_ = 0;
  uint32_t title_until_ = 0;
  uint32_t handshake_start_ = 0;
  uint32_t load_failed_start_ = 0;

  // ---- File List ----
  LyricFileInfo lyric_files_[kMaxLyricFiles];
  const char* visible_lines_[kVisibleLines];
  uint8_t lyric_file_count_ = 0;
  uint8_t selected_file_index_ = 0;
  uint8_t list_scroll_ = 0;

  // ---- State ----
  State state_ = State::FileList;
  bool loaded_ = false;
  bool playing_ = false;
  bool paused_ = false;
  bool new_word_ = false;
  bool display_active_ = false;
  volatile bool audio_started_ = false;

  // ---- Parsing ----
  bool ParseLine(const char* line);
  bool ParseWordTimestamp(const char* text, uint32_t& milliseconds, uint8_t& consumed);
  void UpdateCurrentWord(uint32_t elapsed);

  // ---- File Listing & Loading ----
  void ScanLyricFiles();
  bool LoadLyrics(const char* filename);
  void LoadSelectedLyric();

  // ---- State Transitions ----
  void EnterFileList();
  void EnterLoading();
  void EnterWaitingHandshake();
  void EnterPlaying();
  void EnterPaused();
  void ReturnToFileList();

  // ---- Playback Control ----
  void StartPlayback();
  void PausePlayback();
  void ResumePlayback();
  void StopPlayback();

  // ---- Display ----
  void ShowFileList();
  void ShowStatus(const char* text, TextSize size);
  bool ShowCurrentWord();
  bool ClearDisplay();

  // ---- BLE ----
  void SendPlayCommand(const char* song_name);

  // ---- Helpers ----
  void ClearData();
  const char* GetCurrentWord() const;

  // ---- Input Handlers ----
  bool HandleFileListInput(const InputEvent& event);
  bool HandlePlaybackInput(const InputEvent& event);
};

extern LyricsApp lyrics_app;

#endif