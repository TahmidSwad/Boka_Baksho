#ifndef APP_BASE_H
#define APP_BASE_H

#include <Arduino.h>

#include "common/event_types.h"

// ==========================================================
// APPLICATION INTERFACE
// ==========================================================
// Every application (including the SystemUi launcher) must
// inherit from this interface. The AppManager treats all apps
// uniformly: it owns a registry plus a navigation stack, and
// routes input to the top of the stack.
//
// Lifecycle:
//   Begin()      - one-time init during system startup.
//   OnActivate() - app became the top of the stack (visible,
//                  receives input, may draw).
//   OnDeactivate()- app left the top of the stack (hidden).
//   Update()     - pumped every loop while on top.
//   HandleInput()- input delivered to the top app; return
//                  true when consumed. Returning false for a
//                  Back event pops the navigation stack.
// ==========================================================

class IApp {
public:
  virtual ~IApp() = default;

  // Called once during system startup.
  virtual bool Begin() = 0;

  // Called when the app becomes the top of the stack.
  virtual bool OnActivate() = 0;

  // Called when the app leaves the top of the stack.
  virtual void OnDeactivate() = 0;

  // Called periodically while the app is on top.
  virtual void Update() {}

  // Handle an input event. Return true if consumed.
  virtual bool HandleInput(const InputEvent& event) = 0;

  // Return the app's unique identifier.
  virtual AppId GetAppId() const = 0;

  // Human-readable name shown by the launcher menu.
  virtual const char* GetName() const = 0;
};

#endif