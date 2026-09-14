#ifndef SYSTEM_UI_H
#define SYSTEM_UI_H

#include <Arduino.h>

#include "core/app_base.h"

// ==========================================================
// SYSTEM UI (LAUNCHER)
// ==========================================================
// The root of the navigation stack. Renders the scrolling app
// menu and launches the selected application. Rotating the
// encoder moves the selection, ENTER opens the highlighted
// app. BACK is ignored (the root cannot be popped).
// ==========================================================

class SystemUi : public IApp {
public:
  bool Begin() override;
  bool OnActivate() override;
  void OnDeactivate() override;
  bool HandleInput(const InputEvent& event) override;
  AppId GetAppId() const override { return AppId::SystemUi; }
  const char* GetName() const override { return ""; }

private:
  static constexpr uint8_t kMaxMenuItems = static_cast<uint8_t>(AppId::Count);

  uint8_t selection_ = 0;
  const char* menu_items_[kMaxMenuItems] = {nullptr};

  bool ShowMenu();
};

extern SystemUi system_ui;

#endif