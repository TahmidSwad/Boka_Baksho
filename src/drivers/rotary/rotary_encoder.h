#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <Arduino.h>

// ==========================================================
// ROTARY ENCODER DRIVER (polled, no interrupt)
// ==========================================================
// CLK is sampled in the main loop (Poll).  A rising edge is
// detected by comparing against the previous CLK level; DT
// is then read to decide direction.  Bounces inside a short
// time window are ignored.
// ==========================================================

class RotaryEncoder {
public:
  bool Begin(uint8_t clk_pin, uint8_t dt_pin);
  void Poll();
  int ReadDelta();
  void Reset();

private:
  uint8_t clk_pin_ = 0;
  uint8_t dt_pin_ = 0;
  bool clk_high_ = false;          // previous CLK level
  uint32_t last_edge_us_ = 0;      // last accepted edge (debounce)
  int delta_ = 0;
  bool bootstrapped_ = false;      // first Poll() establishes baseline
};

extern RotaryEncoder rotary_encoder;

#endif