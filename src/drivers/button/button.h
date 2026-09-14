#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

// ==========================================================
// BUTTON DRIVER
// ==========================================================
// A single debounced momentary button (active-low, INPUT_PULLUP).
// Polled from the main loop; a debounced press transition
// latches a flag consumed by WasPressed().
// ==========================================================

class Button {
public:
  void Begin(uint8_t pin);
  void Poll();
  bool WasPressed();   // returns and clears the latched press

private:
  uint8_t pin_ = 0;
  bool raw_ = false;        // raw pin level (last read)
  bool debounced_ = false;  // debounced stable level
  bool latched_ = false;    // pending press event
  uint32_t hold_until_ = 0; // debounce window end
};

#endif