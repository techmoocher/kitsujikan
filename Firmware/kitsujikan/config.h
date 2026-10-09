// config.h: every pin number and timing value lives here.
// Pin numbers are ESP32-C3 GPIO numbers; the XIAO silkscreen label is in the comment.
// They match the schematic (see README "Pin map").
#pragma once
#include <stdint.h>

// ---------------- Pins ----------------
constexpr uint8_t PIN_BTN_30 = 2;   // D0  30-min button (strapping pin, 10k external pull-up)
constexpr uint8_t PIN_BTN_60 = 3;   // D1  60-min button
constexpr uint8_t PIN_BAT    = 4;   // D2  battery voltage, through a 1:2 divider (200k + 200k)
constexpr uint8_t PIN_BTN_90 = 5;   // D3  90-min button
constexpr uint8_t PIN_SDA    = 6;   // D4  OLED SDA
constexpr uint8_t PIN_SCL    = 7;   // D5  OLED SCL
constexpr uint8_t PIN_BTN_NO = 21;  // D6  red button: no / pause (hold = stop)
constexpr uint8_t PIN_BUZZER = 20;  // D7  buzzer transistor base (via 1k)
constexpr uint8_t PIN_BTN_OK = 10;  // D10 green button: ok / continue

constexpr uint8_t OLED_ADDR = 0x3C;  // most 0.96" SSD1306 modules

// ---------------- Timing ----------------
// While testing, set FAST_TEST to 1: every "minute" then lasts 1 second,
// so a 30-min session takes 30 s and the 5-min break takes 5 s.
#define FAST_TEST 0

#if FAST_TEST
constexpr uint32_t MINUTE_MS = 1000UL;
#else
constexpr uint32_t MINUTE_MS = 60UL * 1000UL;
#endif

constexpr uint32_t BREAK_MINUTES     = 5;
constexpr uint32_t PROMPT_TIMEOUT_MS = 5UL * 60UL * 1000UL;  // stop asking after 5 min with no answer
constexpr uint32_t IDLE_SLEEP_MS     = 60UL * 1000UL;        // deep sleep after 1 min idle
constexpr uint32_t DEBOUNCE_MS       = 25;                   // ignore contact bounce shorter than this
constexpr uint32_t LONG_PRESS_MS     = 800;                  // hold red this long = stop
constexpr uint32_t REDRAW_MS         = 250;                  // screen refresh period
constexpr uint32_t BATTERY_SAMPLE_MS = 5000;                 // battery reading period
