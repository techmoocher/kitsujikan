#include "pomodoro.h"

// Note on time math: millis() wraps around to 0 after ~49.7 days.
// Comparing with (int32_t)(a - b) instead of a < b keeps working across the wrap.

Pomodoro::Pomodoro(uint32_t minuteMs, uint32_t breakMinutes, uint32_t promptTimeoutMs)
    : minuteMs_(minuteMs), breakMin_(breakMinutes), promptTimeoutMs_(promptTimeoutMs) {}

void Pomodoro::startPhase(Mode m, uint32_t lengthMs, uint32_t now) {
  mode_ = m;
  phaseLen_ = lengthMs;
  endAt_ = now + lengthMs;
}

void Pomodoro::ask(Mode prompt, uint32_t now) {
  mode_ = prompt;
  promptSince_ = now;
}

void Pomodoro::goIdle(uint32_t now) {
  mode_ = Mode::Idle;
  idleSince_ = now;
}

Alert Pomodoro::startSession(uint8_t minutes, uint32_t now) {
  sessionMin_ = minutes;
  startPhase(Mode::Focus, sessionLengthMs(), now);
  return Alert::Started;
}

uint32_t Pomodoro::remainingMs(uint32_t now) const {
  switch (mode_) {
    case Mode::Focus:
    case Mode::Break: {
      int32_t left = (int32_t)(endAt_ - now);
      return left > 0 ? (uint32_t)left : 0;
    }
    case Mode::Paused:    return pausedLeft_;
    case Mode::AskBreak:  return breakLengthMs();    // what we're offering
    case Mode::AskRepeat: return sessionLengthMs();  // what we're offering
    default:              return 0;
  }
}

Alert Pomodoro::handle(Input in, uint32_t now) {
  const bool isStart = in == Input::Start30 || in == Input::Start60 || in == Input::Start90;
  const uint8_t startMin = in == Input::Start30 ? 30 : in == Input::Start60 ? 60 : 90;

  switch (mode_) {
    case Mode::Idle:
      if (isStart) return startSession(startMin, now);
      return Alert::None;

    case Mode::Focus:
    case Mode::Break:
      // Session buttons are ignored while running so a bump can't restart the timer.
      if (in == Input::No) {
        pausedPhase_ = mode_;
        pausedLeft_ = remainingMs(now);
        mode_ = Mode::Paused;
        return Alert::Paused;
      }
      if (in == Input::NoLong) { goIdle(now); return Alert::Stopped; }
      return Alert::None;

    case Mode::Paused:
      if (in == Input::Ok || in == Input::No) {
        const uint32_t left = pausedLeft_;
        const uint32_t len = phaseLen_;
        startPhase(pausedPhase_, left, now);
        phaseLen_ = len;  // keep the original length so the progress bar stays right
        return Alert::Resumed;
      }
      if (in == Input::NoLong) { goIdle(now); return Alert::Stopped; }
      return Alert::None;

    case Mode::AskBreak:
      if (in == Input::Ok) { startPhase(Mode::Break, breakLengthMs(), now); return Alert::Started; }
      if (in == Input::No) { ask(Mode::AskRepeat, now); return Alert::None; }  // skip the break
      if (isStart) return startSession(startMin, now);
      return Alert::None;

    case Mode::AskRepeat:
      if (in == Input::Ok) return startSession(sessionMin_, now);
      if (in == Input::No) { goIdle(now); return Alert::Stopped; }
      if (isStart) return startSession(startMin, now);
      return Alert::None;
  }
  return Alert::None;
}

Alert Pomodoro::update(uint32_t now) {
  switch (mode_) {
    case Mode::Focus:
      if ((int32_t)(now - endAt_) >= 0) { ask(Mode::AskBreak, now); return Alert::FocusDone; }
      break;
    case Mode::Break:
      if ((int32_t)(now - endAt_) >= 0) { ask(Mode::AskRepeat, now); return Alert::BreakDone; }
      break;
    case Mode::AskBreak:
    case Mode::AskRepeat:
      if (now - promptSince_ >= promptTimeoutMs_) { goIdle(now); return Alert::TimedOut; }
      break;
    default:
      break;
  }
  return Alert::None;
}
