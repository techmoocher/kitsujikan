// button.h: debounced push button with press / short-release / long-press events.
//
// Why debounce? A button's metal contacts "bounce" for a few ms, so one press
// can look like several. We only accept a new state once it has been stable
// for DEBOUNCE_MS. (Background: http://www.ganssle.com/debouncing.htm)
//
// Wiring: button between the pin and GND, internal pull-up on, so
// released = HIGH and pressed = LOW ("active low").
#pragma once
#include <Arduino.h>

class Button {
 public:
  enum Event : uint8_t { NONE, PRESS, SHORT_RELEASE, LONG_PRESS };

  // ignoreHeldAtStart: if the button is already down at boot (e.g. it just woke
  // the chip from deep sleep), swallow that press until it's released.
  void begin(uint8_t pin, bool ignoreHeldAtStart = false);
  Event poll(uint32_t now);  // call every loop; returns at most one event
  bool isDown() const { return stable_; }

 private:
  uint8_t pin_ = 0;
  bool stable_ = false;     // debounced state (true = pressed)
  bool lastRaw_ = false;    // last raw reading
  bool longFired_ = false;  // LONG_PRESS already sent for this press
  bool armed_ = true;       // false = ignore until the next release
  uint32_t changedAt_ = 0;  // when the raw reading last changed
  uint32_t pressedAt_ = 0;  // when the debounced press began
};
