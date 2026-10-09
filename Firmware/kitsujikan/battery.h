// battery.h: reads the LiPo voltage through the 200k/200k divider on D2.
//
// The divider halves the voltage (4.2 V full -> 2.1 V at the pin), which keeps
// it inside the ESP32-C3 ADC's ~0-2.5 V range. analogReadMilliVolts() applies
// the chip's factory calibration, so the result is already in millivolts.
//
// Limitation: the XIAO has no "charging" signal, so while USB is plugged in
// this shows the charger's voltage, not the true battery level.
#pragma once
#include <Arduino.h>

class Battery {
 public:
  void begin(uint8_t pin);
  void update(uint32_t now);              // samples every BATTERY_SAMPLE_MS
  uint16_t millivolts() const { return mv_; }
  uint8_t percent() const;                // rough LiPo state of charge, 0-100

 private:
  uint16_t readOnce();
  uint8_t pin_ = 0;
  uint16_t mv_ = 0;
  uint32_t lastSample_ = 0;
};
