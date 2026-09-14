#ifndef INPUT_TYPES_H
#define INPUT_TYPES_H

#include <Arduino.h>

// ==========================================================
// INPUT EVENT
// ==========================================================
// Hardware input is normalized into a small set of abstract
// events (rotary encoder steps and the two menu buttons).
// Every application interprets them in its own way:
//
//   encoder rotation  -> RotateLeft / RotateRight
//   ENTER button      -> Enter
//   BACK button       -> Back
//
// The AppManager delivers these to the foreground app. If the
// app returns false from HandleInput() for a Back event, the
// AppManager pops the navigation stack (the app is exited and
// the launcher takes over again).
// ==========================================================

enum class InputType : uint8_t {
  RotateLeft,
  RotateRight,
  Enter,
  Back

  ButtonIncrement,
  ButtonDecrement

};

struct InputEvent {
  InputType type;
};

#endif