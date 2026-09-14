#include "drivers/button/button.h"

void Button::Begin(uint8_t pin) {
  pin_ = pin;
  pinMode(pin_, INPUT_PULLUP);
  raw_ = (digitalRead(pin_) == LOW);
  debounced_ = raw_;
  latched_ = false;
  hold_until_ = 0;
}

void Button::Poll() {
  bool pressed = (digitalRead(pin_) == LOW);

  if (pressed != raw_) {
    raw_ = pressed;
    hold_until_ = millis() + 20;   // debounce window
  }

  if (millis() < hold_until_) return;

  if (pressed != debounced_) {
    debounced_ = pressed;
    if (pressed) latched_ = true;
  }
}

bool Button::WasPressed() {
  if (latched_) {
    latched_ = false;
    return true;
  }
  return false;
}