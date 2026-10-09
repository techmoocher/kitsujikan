// Laptop tests for the timer logic: no board needed.
// Run from the Firmware/ folder:
//   g++ -std=c++17 -Wall -Ikitsujikan test/test_pomodoro.cpp kitsujikan/pomodoro.cpp -o test_pomodoro && ./test_pomodoro
//
// Because pomodoro.cpp has no Arduino code, we can drive it with fake times
// and check every transition. Catching logic bugs here is much faster than
// flashing the board and waiting 30 minutes.
#include <cstdio>
#include <cstring>
#include "pomodoro.h"
#include "timefmt.h"

static int failures = 0;
#define CHECK(cond)                                                     \
  do {                                                                  \
    if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } \
  } while (0)

static const uint32_t MIN = 60000;  // real minutes in ms

static bool clockIs(uint32_t ms, const char* expected) {
  char buf[12];
  formatClock(ms, buf, sizeof(buf));
  if (strcmp(buf, expected) != 0) printf("  formatClock(%u) = %s, expected %s\n", ms, buf, expected);
  return strcmp(buf, expected) == 0;
}

int main() {
  // --- time formatting ---
  CHECK(clockIs(30 * MIN, "30:00"));
  CHECK(clockIs(30 * MIN - 1, "30:00"));     // rounds up: still shows 30:00
  CHECK(clockIs(59 * MIN + 59000, "59:59"));
  CHECK(clockIs(60 * MIN, "01:00:00"));      // hours appear at 1 h
  CHECK(clockIs(90 * MIN, "01:30:00"));
  CHECK(clockIs(1, "00:01"));
  CHECK(clockIs(0, "00:00"));

  // --- full happy path: 30 min -> break -> repeat ---
  {
    Pomodoro p(MIN, 5, 5 * MIN);
    uint32_t t = 1000;
    CHECK(p.mode() == Mode::Idle);
    CHECK(p.handle(Input::Ok, t) == Alert::None);          // OK does nothing while idle
    CHECK(p.handle(Input::Start30, t) == Alert::Started);
    CHECK(p.mode() == Mode::Focus);
    CHECK(p.remainingMs(t) == 30 * MIN);
    CHECK(p.handle(Input::Start90, t + 5) == Alert::None);  // ignored while running
    CHECK(p.update(t + 30 * MIN - 1) == Alert::None);
    CHECK(p.update(t + 30 * MIN) == Alert::FocusDone);
    CHECK(p.mode() == Mode::AskBreak);
    CHECK(p.remainingMs(t + 30 * MIN) == 5 * MIN);         // offers a 5:00 break
    t += 30 * MIN + 2000;
    CHECK(p.handle(Input::Ok, t) == Alert::Started);
    CHECK(p.mode() == Mode::Break);
    CHECK(p.update(t + 5 * MIN) == Alert::BreakDone);
    CHECK(p.mode() == Mode::AskRepeat);
    CHECK(p.remainingMs(t + 5 * MIN) == 30 * MIN);         // offers the same 30 min
    CHECK(p.handle(Input::Ok, t + 5 * MIN + 10) == Alert::Started);
    CHECK(p.mode() == Mode::Focus);
    CHECK(p.sessionMinutes() == 30);
  }

  // --- pause keeps the remaining time ---
  {
    Pomodoro p(MIN, 5, 5 * MIN);
    p.handle(Input::Start60, 0);
    CHECK(p.handle(Input::No, 10 * MIN) == Alert::Paused);
    CHECK(p.mode() == Mode::Paused);
    CHECK(p.remainingMs(10 * MIN) == 50 * MIN);
    CHECK(p.update(70 * MIN) == Alert::None);               // time doesn't run while paused
    CHECK(p.remainingMs(70 * MIN) == 50 * MIN);
    CHECK(p.handle(Input::Ok, 70 * MIN) == Alert::Resumed);
    CHECK(p.mode() == Mode::Focus);
    CHECK(p.remainingMs(70 * MIN) == 50 * MIN);
    CHECK(p.phaseLengthMs() == 60 * MIN);                   // progress bar still based on 60 min
    CHECK(p.update(120 * MIN) == Alert::FocusDone);
  }

  // --- hold red = stop ---
  {
    Pomodoro p(MIN, 5, 5 * MIN);
    p.handle(Input::Start90, 0);
    CHECK(p.handle(Input::NoLong, 1000) == Alert::Stopped);
    CHECK(p.mode() == Mode::Idle);
    CHECK(p.idleSince() == 1000);
  }

  // --- saying no: skip break -> asked to repeat -> no -> idle ---
  {
    Pomodoro p(MIN, 5, 5 * MIN);
    p.handle(Input::Start30, 0);
    p.update(30 * MIN);
    CHECK(p.handle(Input::No, 30 * MIN + 1) == Alert::None);
    CHECK(p.mode() == Mode::AskRepeat);
    CHECK(p.handle(Input::No, 30 * MIN + 2) == Alert::Stopped);
    CHECK(p.mode() == Mode::Idle);
  }

  // --- a session button during a prompt starts that session instead ---
  {
    Pomodoro p(MIN, 5, 5 * MIN);
    p.handle(Input::Start30, 0);
    p.update(30 * MIN);
    CHECK(p.handle(Input::Start90, 30 * MIN + 5) == Alert::Started);
    CHECK(p.mode() == Mode::Focus);
    CHECK(p.sessionMinutes() == 90);
  }

  // --- unanswered prompt times out to idle ---
  {
    Pomodoro p(MIN, 5, 5 * MIN);
    p.handle(Input::Start30, 0);
    p.update(30 * MIN);
    CHECK(p.update(35 * MIN - 1) == Alert::None);
    CHECK(p.update(35 * MIN) == Alert::TimedOut);
    CHECK(p.mode() == Mode::Idle);
  }

  // --- millis() wrap-around (~49.7 days) doesn't break timing ---
  {
    Pomodoro p(MIN, 5, 5 * MIN);
    const uint32_t start = 0xFFFFFFFFu - 10 * MIN;  // 10 min before the wrap
    p.handle(Input::Start30, start);
    CHECK(p.update(start + 20 * MIN) == Alert::None);       // past the wrap, 10 min left
    CHECK(p.remainingMs(start + 20 * MIN) == 10 * MIN);
    CHECK(p.update(start + 30 * MIN) == Alert::FocusDone);
  }

  if (failures == 0) printf("All tests passed.\n");
  return failures == 0 ? 0 : 1;
}
