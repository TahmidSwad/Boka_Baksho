#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <Arduino.h>

#include "core/app_base.h"
#include "core/event_bus.h"

// ==========================================================
// APPLICATION MANAGER
// ==========================================================
// Maintains the registry of every app and a navigation stack.
// The bottom of the stack is always the SystemUi launcher, so
// there is always a foreground app. Launch = push, Back = pop.
// Input events are routed to the top of the stack; if the top
// app does not consume a Back event, the stack is popped.
//
// This is a genuinely lightweight, statically allocated
// design: no dynamic allocation anywhere.
// ==========================================================

class AppManager : public IEventSubscriber {
public:
  // Registry.
  bool RegisterApp(IApp* app);
  uint8_t GetAppCount() const;
  IApp* GetAppAt(uint8_t index) const;

  // Navigation.
  bool Boot(IApp* system_ui);           // stack = [ system_ui ]
  bool LaunchApp(AppId id);             // push an app onto the stack
  void GoBack();                        // pop the top app (never the root)

  // State queries.
  bool IsForeground(AppId id) const;    // used by DisplayService ownership
  IApp* GetForeground() const;

  // Loop pump.
  void Update();                        // only pumps the foreground app

  // Event routing (IEventSubscriber).
  void OnEvent(const Event& event) override;

private:
  static constexpr uint8_t kMaxApps = static_cast<uint8_t>(AppId::Count);
  static constexpr uint8_t kMaxDepth = 8;

  IApp* apps_[kMaxApps] = {nullptr};
  uint8_t app_count_ = 0;

  IApp* stack_[kMaxDepth] = {nullptr};
  uint8_t depth_ = 0;

  bool RouteInput(const InputEvent& event);
  bool IsRegistered(AppId id) const;
};

extern AppManager app_manager;

#endif