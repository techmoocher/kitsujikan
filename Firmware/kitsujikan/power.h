// power.h: deep sleep and waking up.
//
// In deep sleep almost the whole chip is off (~44 uA on the XIAO ESP32-C3).
// Only GPIO0-5 can wake it, which is why the 30/60/90 buttons are on
// GPIO2/3/5. Waking from deep sleep is a fresh boot: setup() runs again,
// and powerWakePin() tells us which button did it.
#pragma once
#include <stdint.h>

constexpr uint8_t NO_WAKE_PIN = 0xFF;

uint8_t powerWakePin();    // GPIO that woke us, or NO_WAKE_PIN (normal power-on)
[[noreturn]] void powerDeepSleep();
