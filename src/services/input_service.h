#ifndef INPUT_SERVICE_H
#define INPUT_SERVICE_H

#include <Arduino.h>

#include "common/input_types.h"
#include "drivers/rotary/rotary_encoder.h"
#include "drivers/button/button.h"

// ==========================================================
// INPUT SERVICE
// ==========================================================
// Scans the rotary encoder and the two menu buttons and posts
// normalized InputEvents onto the EventBus. Also provides a
// serial debug input (l / r / e / b) so the system can be
// driven without hardware attached.
// ==========================================================

class InputService {
public:
  bool Begin();
  void Poll();
  void PollSerialDebug();

private:
  void PostInput(InputType type);

  Button increment_button;
  Button decrement_button;

};

extern InputService input_service;

#endif