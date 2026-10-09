#include "battery.h"
#include "config.h"

void Battery::begin(uint8_t pin) {
  pin_ = pin;
  pinMode(pin_, INPUT);
  analogSetPinAttenuation(pin_, ADC_11db);  // widest input range (~0-2.5 V on the C3)
  mv_ = readOnce();
  lastSample_ = millis();
}

uint16_t Battery::readOnce() {
  uint32_t sum = 0;
  for (int i = 0; i < 8; i++) sum += analogReadMilliVolts(pin_);  // average out noise
  return (uint16_t)(sum / 8 * 2);  // x2 undoes the divider
}

void Battery::update(uint32_t now) {
  if (now - lastSample_ < BATTERY_SAMPLE_MS) return;
  lastSample_ = now;
  // Smooth readings (3/4 old + 1/4 new) so the percentage doesn't flicker.
  mv_ = (uint16_t)((mv_ * 3u + readOnce()) / 4u);
}

uint8_t Battery::percent() const {
  // A LiPo's voltage isn't linear with charge; this table is a typical curve.
  static const uint16_t mv[]  = {3300, 3500, 3650, 3750, 3850, 3950, 4100, 4200};
  static const uint8_t  pct[] = {   0,    5,   15,   35,   55,   75,   92,  100};
  const int n = sizeof(mv) / sizeof(mv[0]);
  if (mv_ <= mv[0]) return 0;
  if (mv_ >= mv[n - 1]) return 100;
  for (int i = 1; i < n; i++) {
    if (mv_ <= mv[i]) {  // straight line between the two nearest table points
      return pct[i - 1] + (uint32_t)(mv_ - mv[i - 1]) * (pct[i] - pct[i - 1]) / (mv[i] - mv[i - 1]);
    }
  }
  return 100;
}
