#ifndef STOPWATCH_APP_H
#define STOPWATCH_APP_H

#include <Arduino.h>

#include "core/app_base.h"

// ==========================================================
// STOPWATCH / TIMER APP
// ==========================================================
// Two modes selected by the navigation buttons:
//   timer_set_min_ == 0  -> STOPWATCH (counts up from zero)
//   timer_set_min_  > 0   -> TIMER    (counts down from the preset)
//
// Rotating the encoder (when stopped) sets the timer preset,
// one minute per click, and cannot go below zero.  When the
// preset is zero the app is a stopwatch.
//
// ENTER : start / stop (pause-resume) toggle.
// BACK  : full reset.  Exits to the launcher only when the
//         app is already in its reset state (stopwatch at 0).
//
// When a running timer reaches zero the display flashes until
// the user presses ENTER (reset to preset) or BACK (full reset).
// ==========================================================

class StopwatchApp : public IApp {
public:
  bool Begin() override;
  bool OnActivate() override;
  void OnDeactivate() override;
  void Update() override;
  bool HandleInput(const InputEvent& event) override;
  AppId GetAppId() const override { return AppId::Clock; }
  const char* GetName() const override { return "Timer"; }

private:
  static constexpr uint32_t kMinuteMs = 60UL * 1000UL;
  static constexpr uint32_t kBlinkMs = 500;
  static constexpr uint32_t kDrawIntervalMs = 200;

  enum class Mode { STOPWATCH, TIMER };

  Mode mode_ = Mode::STOPWATCH;
  bool running_ = false;
  bool finished_ = false;
  bool blink_on_ = true;

  uint16_t timer_set_min_ = 0;     // preset in whole minutes (0 == stopwatch)
  uint32_t elapsed_ms_ = 0;        // stopwatch accumulated time
  uint32_t remaining_ms_ = 0;      // timer countdown

  uint32_t last_tick_ms_ = 0;
  uint32_t last_draw_ms_ = 0;
  uint32_t last_blink_ms_ = 0;

  char time_buf_[12];

  void Start();
  void ResetAll();
  bool IsReset() const;
  void Draw();
  void FormatTime(char* buf, size_t len, uint32_t ms) const;
};

extern StopwatchApp stopwatch_app;

#endif