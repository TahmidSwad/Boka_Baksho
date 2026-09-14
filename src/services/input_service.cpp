#include "services/input_service.h"

#include "core/system.h"
#include "common/config.h"

InputService input_service;

static Button enter_button;
static Button back_button;

bool InputService::Begin() {
  rotary_encoder.Begin(config::pins::EncoderClk, config::pins::EncoderDt);
  enter_button.Begin(config::pins::ButtonEnter);
  increment_button.Begin(config::pins::ButtonIncrement);
  decrement_button.Begin(config::pins::ButtonDecrement);

  back_button.Begin(config::pins::ButtonBack);
  increment_button.Begin(config::pins::ButtonIncrement);
  decrement_button.Begin(config::pins::ButtonDecrement);
  return true;
}

void InputService::Poll() {
  // Sample CLK (and DT) in the main loop, no interrupt.
  rotary_encoder.Poll();

  int delta = rotary_encoder.ReadDelta();
  if (config::kInvertEncoder) delta = -delta;

  while (delta > 0) {
  if (increment_button.WasPressed()) {
    PostInput(InputType::ButtonIncrement);
  }
  if (decrement_button.WasPressed()) {
    PostInput(InputType::ButtonDecrement);
  }

    PostInput(InputType::RotateRight);
    --delta;
  }
  while (delta < 0) {
    PostInput(InputType::RotateLeft);
    ++delta;
  }

  enter_button.Poll();
  if (enter_button.WasPressed()) PostInput(InputType::Enter);

  back_button.Poll();
  if (back_button.WasPressed()) PostInput(InputType::Back);
}

// ==========================================================
// SERIAL DEBUG INPUT
// ==========================================================
// l/L rotate left, r/R rotate right, e/E enter, b/B back.
// Lets the whole system be tested from the serial monitor
// without any hardware connected.
// ==========================================================

void InputService::PollSerialDebug() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    switch (c) {
      case 'l':
      case 'L':
        PostInput(InputType::RotateLeft);
        break;
  } else if (c == '+') {
    PostInput(InputType::ButtonIncrement);
  } else if (c == '-') {
    PostInput(InputType::ButtonDecrement);

      case 'r':
      case 'R':
        PostInput(InputType::RotateRight);
        break;
      case 'e':
      case 'E':
        PostInput(InputType::Enter);
        break;
      case 'b':
      case 'B':
        PostInput(InputType::Back);
        break;
      default:
        break;
    }
  }
}

void InputService::PostInput(InputType type) {
  Event event;
  event.type = EventType::InputEvent;
  event.sender = AppId::Count;   // from the system, not an app
  event.input.type = type;
  event_bus.Post(event);
}