#ifndef SYSTEM_UI_H
#define SYSTEM_UI_H

#include <Arduino.h>

#include "core/app_base.h"

// ==========================================================
// SYSTEM UI (LAUNCHER)
// ==========================================================
// The root of the navigation stack. Shows an animated cat
// screensaver on activation. Any button press transitions to
// the scrollable app menu. Rotating the encoder scrolls
// between apps, ENTER launches the highlighted app.
// BACK is ignored (the root cannot be popped).
// ==========================================================

class SystemUi : public IApp {
public:
  bool Begin() override;
  bool OnActivate() override;
  void OnDeactivate() override;
  bool HandleInput(const InputEvent& event) override;
  void Update() override;
  AppId GetAppId() const override { return AppId::SystemUi; }
  const char* GetName() const override { return ""; }

private:
  static constexpr uint8_t kMaxMenuItems = static_cast<uint8_t>(AppId::Count);

  enum class CatState : uint8_t {
    Happy,
    Blink,
    Alert,
    Neutral,
    Sad,
    Sleepy,
    Angry
  };

  // Menu state.
  uint8_t selection_ = 0;
  const char* menu_items_[kMaxMenuItems] = {nullptr};

  // Screensaver state.
  bool screensaver_active_ = true;
  CatState base_state_ = CatState::Neutral;
  CatState draw_state_ = CatState::Neutral;
  int8_t cat_y_offset_ = 0;

  unsigned long hold_start_ = 0;
  unsigned long hold_duration_ = 0;

  unsigned long next_blink_time_ = 0;
  bool is_blinking_ = false;
  unsigned long blink_end_time_ = 0;

  bool sleepy_mode_ = false;

  // Look direction: 0=center, -1=left, +1=right
  int8_t look_dir_ = 0;
  unsigned long look_end_time_ = 0;
  unsigned long next_look_time_ = 0;
  bool is_looking_ = false;
  unsigned long next_neutral_blink_ = 0;
  bool in_expression_ = false;

  void PickNextExpression();
  bool ShowScreensaver();
  bool ShowMenu();
};

extern SystemUi system_ui;

#endif
