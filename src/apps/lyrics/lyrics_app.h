#ifndef LYRICS_APP_H
#define LYRICS_APP_H

#include <Arduino.h>

#include "core/app_base.h"

class LyricsApp : public IApp {
public:
  bool Begin() override;
  bool OnActivate() override;
  void OnDeactivate() override;
  void Update() override;
  bool HandleInput(const InputEvent& event) override;
  AppId GetAppId() const override { return AppId::Lyrics; }
  const char* GetName() const override { return "Lyrics"; }

private:
  static constexpr uint16_t kMaxWords = 1000;
  static constexpr uint8_t kMaxWordLength = 32;
  static constexpr uint32_t kWordDisplayTimeoutMs = 5000;
  static constexpr uint32_t kTitleTimeoutMs = 1500;

  struct LyricWord {
    uint32_t timestamp;
    char word[kMaxWordLength];
  };

  LyricWord words_[kMaxWords];
  uint16_t word_count_ = 0;
  uint16_t current_word_ = 0;

  uint32_t start_time_ = 0;          // timeline anchor (ms)
  uint32_t pause_time_ = 0;          // when pause started (ms)
  uint32_t word_display_start_ = 0;  // when the current word was shown
  uint32_t title_until_ = 0;         // status text remains until this ms

  uint8_t file_index_ = 0;

  bool loaded_ = false;
  bool playing_ = false;
  bool paused_ = false;
  bool new_word_ = false;
  bool display_active_ = false;

  bool ParseLine(const char* line);
  bool ParseWordTimestamp(const char* text, uint32_t& milliseconds,
                          uint8_t& consumed);
  void UpdateCurrentWord(uint32_t elapsed);
  bool LoadLyrics(const char* filename);
  void Start();
  void TogglePlayback();
  void SwitchFile(int8_t dir);
  bool ShowCurrentWord();
  bool ShowStatus(const char* text, TextSize size);
  bool ClearDisplay();
  void ClearData();
  const char* GetCurrentWord() const;
};

extern LyricsApp lyrics_app;

#endif