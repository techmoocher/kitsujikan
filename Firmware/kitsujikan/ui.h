// ui.h: everything drawn on the 128x64 OLED.
//
// Screen layout:
//   y 0-7    header: what's happening + battery
//   y 14-37  the big clock (or a prompt)
//   y 44-47  progress bar
//   y 56-63  footer: what the buttons do right now
#pragma once
#include <stdint.h>
#include "pomodoro.h"

bool uiBegin();  // false if the OLED didn't answer on I2C
void uiDraw(const Pomodoro& timer, uint32_t now, uint8_t batteryPercent);
void uiMessage(const char* line1, const char* line2);  // simple two-line screen
void uiOff();    // panel off before deep sleep
