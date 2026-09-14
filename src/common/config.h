#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================================
// GLOBAL CONFIGURATION
// ==========================================================

namespace config {

// ==========================================================
// HARDWARE PINS
// ==========================================================
namespace pins {
  // Rotary encoder (quadrature CLK / DT signals).
  constexpr uint8_t EncoderClk = 26;
  constexpr uint8_t EncoderDt = 25;

  // Menu buttons (INPUT_PULLUP, connect to GND).
  constexpr uint8_t ButtonEnter = 27;
  constexpr uint8_t ButtonBack = 14;

  // Increment and Decrement buttons (INPUT_PULLUP, connect to GND).
  constexpr uint8_t ButtonIncrement = 32;
  constexpr uint8_t ButtonDecrement = 33;


  // OLED display (I2C: SDA / SCL).
  constexpr uint8_t OledSda = 21;
  constexpr uint8_t OledScl = 22;
}

// ==========================================================
// OLED DISPLAY
// ==========================================================
namespace display {
  // Preferred I2C address of the SSD1306 (7-bit form).
  constexpr uint8_t I2cAddr = 0x3C;
  // Fallback for clones that use the alternate address.
  constexpr uint8_t I2cAddrFallback = 0x3D;
}

// Flip to true if the encoder direction feels inverted.
constexpr bool kInvertEncoder = false;

// ==========================================================
// LYRICS
// ==========================================================
constexpr const char* kLyricsPathA = "/lyrics/Tumi.txt";
constexpr const char* kLyricsPathB = "/lyrics/Closer.txt";

}  // namespace config

#endif