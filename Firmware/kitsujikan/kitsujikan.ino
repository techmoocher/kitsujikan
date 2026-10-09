// Kitsujikan: rechargeable Pomodoro timer (Seeed XIAO ESP32-C3)
// https://github.com/techmoocher/kitsujikan
//
// How the code is organised (each file has more notes at the top):
//   config.h      pins and timings: change things here first
//   pomodoro.*    the timer logic (state machine), no hardware code
//   button.*      debounced buttons with short / long presses
//   buzzer.*      non-blocking melodies on the passive buzzer
//   battery.*     battery voltage and percentage
//   ui.*          everything drawn on the OLED
//   power.*       deep sleep and wake-up
//
// The loop never blocks (no long delay()s): every pass it checks the buttons,
// advances the timer, plays sound and redraws when needed. That's why a
// button press is noticed immediately even while a melody is playing.

#include <Wire.h>
#include "config.h"
#include "pomodoro.h"
#include "button.h"
#include "buzzer.h"
#include "battery.h"
#include "ui.h"
#include "power.h"

Pomodoro timer(MINUTE_MS, BREAK_MINUTES, PROMPT_TIMEOUT_MS);
Button btn30, btn60, btn90, btnOk, btnNo;
Buzzer buzzer;
Battery battery;

uint32_t lastDraw = 0;
bool redrawNow = true;

// Play the sound that matches what the timer just told us.
void react(Alert a) {
  switch (a) {
    case Alert::Started:   buzzer.play(Sounds::START, Sounds::START_LEN); break;
    case Alert::FocusDone: buzzer.play(Sounds::FOCUS_DONE, Sounds::FOCUS_DONE_LEN); break;
    case Alert::BreakDone: buzzer.play(Sounds::BREAK_DONE, Sounds::BREAK_DONE_LEN); break;
    case Alert::Stopped:   buzzer.play(Sounds::STOP, Sounds::STOP_LEN); break;
    case Alert::Paused:
    case Alert::Resumed:   buzzer.play(Sounds::CLICK, Sounds::CLICK_LEN); break;
    default: break;
  }
  if (a != Alert::None) {
    redrawNow = true;
    Serial.printf("[%lu] alert %u -> mode %u\n", (unsigned long)millis(), (unsigned)a, (unsigned)timer.mode());
  }
}

void send(Input in) { react(timer.handle(in, millis())); }

void setup() {
  setCpuFrequencyMhz(80);  // the timer doesn't need 160 MHz; this saves battery
  Serial.begin(115200);

  const uint8_t wakePin = powerWakePin();

  // A button that woke us is still held: start its session now and
  // ignore that same press until it's released (otherwise it counts twice).
  btn30.begin(PIN_BTN_30, true);
  btn60.begin(PIN_BTN_60, true);
  btn90.begin(PIN_BTN_90, true);
  btnOk.begin(PIN_BTN_OK);
  btnNo.begin(PIN_BTN_NO);

  buzzer.begin(PIN_BUZZER);
  battery.begin(PIN_BAT);

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);  // fast I2C: a full-screen update takes ~25 ms
  if (!uiBegin()) Serial.println("OLED not found: check wiring and I2C address (0x3C?)");

  if      (wakePin == PIN_BTN_30) send(Input::Start30);
  else if (wakePin == PIN_BTN_60) send(Input::Start60);
  else if (wakePin == PIN_BTN_90) send(Input::Start90);
  else uiMessage("Kitsujikan", "ready");
}

void loop() {
  const uint32_t now = millis();

  // 1. Buttons
  if (btn30.poll(now) == Button::PRESS) send(Input::Start30);
  if (btn60.poll(now) == Button::PRESS) send(Input::Start60);
  if (btn90.poll(now) == Button::PRESS) send(Input::Start90);
  if (btnOk.poll(now) == Button::PRESS) send(Input::Ok);
  switch (btnNo.poll(now)) {  // red: short tap = no/pause, hold = stop
    case Button::SHORT_RELEASE: send(Input::No); break;
    case Button::LONG_PRESS:    send(Input::NoLong); break;
    default: break;
  }

  // 2. Timer (finishes sessions/breaks, times out prompts)
  react(timer.update(now));

  // 3. Sound and battery
  buzzer.update(now);
  battery.update(now);

  // 4. Screen
  if (redrawNow || now - lastDraw >= REDRAW_MS) {
    uiDraw(timer, now, battery.percent());
    lastDraw = now;
    redrawNow = false;
  }

  // 5. Sleep when nothing has happened for a while
  if (timer.mode() == Mode::Idle && !buzzer.busy() && now - timer.idleSince() >= IDLE_SLEEP_MS) {
    uiMessage("zzz", "press 30/60/90 to wake");
    delay(1500);
    uiOff();
    powerDeepSleep();
  }

  delay(5);  // a short breather; the loop still runs ~150 times per second
}
