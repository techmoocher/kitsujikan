#include "button.h"
#include "config.h"

void Button::begin(uint8_t pin, bool ignoreHeldAtStart) {
  pin_ = pin;
  pinMode(pin_, INPUT_PULLUP);
  stable_ = lastRaw_ = (digitalRead(pin_) == LOW);
  changedAt_ = pressedAt_ = millis();
  armed_ = !(ignoreHeldAtStart && stable_);
}

Button::Event Button::poll(uint32_t now) {
  const bool raw = (digitalRead(pin_) == LOW);
  if (raw != lastRaw_) {  // still bouncing (or a real change): restart the timer
    lastRaw_ = raw;
    changedAt_ = now;
  }

  if (raw != stable_ && now - changedAt_ >= DEBOUNCE_MS) {
    stable_ = raw;
    if (stable_) {  // just pressed
      pressedAt_ = now;
      longFired_ = false;
      return armed_ ? PRESS : NONE;
    }
    // just released
    if (!armed_) { armed_ = true; return NONE; }
    return longFired_ ? NONE : SHORT_RELEASE;
  }

  if (stable_ && armed_ && !longFired_ && now - pressedAt_ >= LONG_PRESS_MS) {
    longFired_ = true;
    return LONG_PRESS;
  }
  return NONE;
}
