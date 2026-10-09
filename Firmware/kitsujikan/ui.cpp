#include "ui.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "timefmt.h"

static Adafruit_SSD1306 display(128, 64, &Wire, -1);  // -1: the module has no reset pin

bool uiBegin() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) return false;
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
  display.clearDisplay();
  display.display();
  return true;
}

// Print text horizontally centered at row y with the given text size.
static void printCentered(const char* text, int16_t y, uint8_t size) {
  int16_t x1, y1;
  uint16_t w, h;
  display.setTextSize(size);
  display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  display.setCursor((128 - (int16_t)w) / 2, y);
  display.print(text);
}

// Big clock: size 3 (18 px per char) fits "MM:SS"; "HH:MM:SS" needs size 2.
static void drawClock(uint32_t ms, int16_t y) {
  char buf[12];
  formatClock(ms, buf, sizeof(buf));
  const uint8_t size = (strlen(buf) * 6 * 3 <= 128) ? 3 : 2;
  printCentered(buf, size == 3 ? y : y + 4, size);
}

static void drawBattery(uint8_t pct) {
  // Outline at the top-right, fill proportional to charge, then the number.
  const int16_t x = 110, y = 0;
  display.drawRect(x, y, 16, 8, SSD1306_WHITE);
  display.fillRect(x + 16, y + 2, 2, 4, SSD1306_WHITE);  // the "nub"
  display.fillRect(x + 2, y + 2, (int16_t)(12 * pct / 100), 4, SSD1306_WHITE);
  char buf[6];
  snprintf(buf, sizeof(buf), "%u%%", pct);
  display.setTextSize(1);
  display.setCursor(x - 6 * (int16_t)strlen(buf) - 2, y);
  display.print(buf);
}

static void drawProgress(uint32_t remaining, uint32_t total) {
  display.drawRect(4, 44, 120, 4, SSD1306_WHITE);
  if (total == 0) return;
  const uint32_t done = total - (remaining > total ? total : remaining);
  display.fillRect(4, 44, (int16_t)(120ULL * done / total), 4, SSD1306_WHITE);
}

void uiDraw(const Pomodoro& t, uint32_t now, uint8_t batteryPercent) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  char line[24];
  const uint32_t left = t.remainingMs(now);

  switch (t.mode()) {
    case Mode::Idle:
      display.print("KITSUJIKAN");
      printCentered("30  60  90", 18, 2);
      printCentered("press to start", 40, 1);
      printCentered("pick a session", 56, 1);
      break;

    case Mode::Focus:
      snprintf(line, sizeof(line), "FOCUS %u", t.sessionMinutes());
      display.print(line);
      drawClock(left, 16);
      drawProgress(left, t.phaseLengthMs());
      printCentered("RED pause  hold:stop", 56, 1);
      break;

    case Mode::Break:
      display.print("BREAK");
      drawClock(left, 16);
      drawProgress(left, t.phaseLengthMs());
      printCentered("RED pause  hold:stop", 56, 1);
      break;

    case Mode::Paused:
      display.print(t.pausedPhase() == Mode::Break ? "BREAK - PAUSED" : "PAUSED");
      if ((now / 500) % 2 == 0) drawClock(left, 16);  // blink the time while paused
      drawProgress(left, t.phaseLengthMs());
      printCentered("GREEN go  hold:stop", 56, 1);
      break;

    case Mode::AskBreak:
      display.print("SESSION DONE!");
      printCentered("Take a break?", 16, 1);
      drawClock(left, 28);
      printCentered("GREEN yes   RED no", 56, 1);
      break;

    case Mode::AskRepeat:
      display.print("BREAK OVER");
      snprintf(line, sizeof(line), "Again: %u min?", t.sessionMinutes());
      printCentered(line, 16, 1);
      drawClock(left, 28);
      printCentered("GREEN yes   RED no", 56, 1);
      break;
  }

  drawBattery(batteryPercent);
  display.display();
}

void uiMessage(const char* line1, const char* line2) {
  display.clearDisplay();
  printCentered(line1, 16, 2);
  printCentered(line2, 44, 1);
  display.display();
}

void uiOff() {
  display.clearDisplay();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);
}
