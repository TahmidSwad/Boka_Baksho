// ==========================================================
// Rotary Encoder Driver — polled, no interrupt
// ==========================================================
// CLK is sampled in the main loop (Poll()). A rising edge is
// detected by comparing against the previous CLK level; DT
// is then read to decide direction. Bounces inside a short
// time window are ignored.
//
// The first Poll() call establishes the baseline (no edge
// counted) so that a stale CLK snapshot at boot never
// produces a spurious first rotation.
// ==========================================================

#include "drivers/rotary/rotary_encoder.h"

// ==========================================================
// GLOBAL INSTANCE
// ==========================================================

RotaryEncoder rotary_encoder;

// ==========================================================
// DEBOUNCE
// ==========================================================
namespace {
constexpr uint32_t kDebounceUs = 1000;
}  // namespace

bool RotaryEncoder::Begin(uint8_t clk_pin, uint8_t dt_pin) {
  clk_pin_ = clk_pin;
  dt_pin_ = dt_pin;

  pinMode(clk_pin_, INPUT);
  pinMode(dt_pin_, INPUT);

  // Don't snapshot here — do it on the first Poll() to avoid
  // a false edge if the encoder state at boot doesn't match
  // the first read.
  clk_high_ = false;
  delta_ = 0;
  last_edge_us_ = 0;
  bootstrapped_ = false;
  return true;
}

// ==========================================================
// POLL
// ==========================================================
// Call from the main loop.  Detects a CLK rising edge and
// reads DT for direction.
//
// The first call establishes the baseline (no edge counted).
// ==========================================================

void RotaryEncoder::Poll() {
  bool clk = (digitalRead(clk_pin_) == HIGH);
  
  if (!bootstrapped_) {
    // First call: establish baseline, don't count anything.
    clk_high_ = clk;
    bootstrapped_ = true;
    return;
  }
  
  // Rising edge: CLK was low, now high.
  if (clk && !clk_high_) {
    uint32_t now = micros();
    if (now - last_edge_us_ >= kDebounceUs) {
      last_edge_us_ = now;
      delta_ += digitalRead(dt_pin_) ? +1 : -1;
    }
  }
  
  clk_high_ = clk;
}

int RotaryEncoder::ReadDelta() {
  int d = delta_;
  delta_ = 0;
  return d;
}

void RotaryEncoder::Reset() {
  delta_ = 0;
}
