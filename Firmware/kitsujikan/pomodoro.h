// pomodoro.h: the timer's brain, written as a "state machine".
//
// Pure C++ with no Arduino calls. It never reads buttons or the clock itself:
// you tell it what happened (handle) and what time it is (update). That keeps
// it testable on a laptop (see Firmware/test/).
//
//   Idle --30/60/90--> Focus --time up--> AskBreak --OK--> Break --time up--> AskRepeat
//                        |  ^                 | NO                               | OK -> Focus (same length)
//                    red |  | red/green       v                                  | NO -> Idle
//                        v  |               AskRepeat
//                       Paused
//   Holding red (NoLong) in Focus/Break/Paused stops everything -> Idle.
//   A prompt with no answer for PROMPT_TIMEOUT_MS -> Idle.
#pragma once
#include <stdint.h>

enum class Mode : uint8_t { Idle, Focus, Break, Paused, AskBreak, AskRepeat };

enum class Input : uint8_t { Start30, Start60, Start90, Ok, No, NoLong };

// Things the rest of the firmware may want to react to (sounds, logging).
enum class Alert : uint8_t { None, Started, FocusDone, BreakDone, Paused, Resumed, Stopped, TimedOut };

class Pomodoro {
 public:
  Pomodoro(uint32_t minuteMs, uint32_t breakMinutes, uint32_t promptTimeoutMs);

  Alert handle(Input in, uint32_t now);  // a button was pressed
  Alert update(uint32_t now);            // call every loop; finishes phases and prompts

  Mode mode() const { return mode_; }
  Mode pausedPhase() const { return pausedPhase_; }  // Focus or Break, valid while Paused
  uint8_t sessionMinutes() const { return sessionMin_; }
  uint32_t phaseLengthMs() const { return phaseLen_; }
  uint32_t remainingMs(uint32_t now) const;
  uint32_t breakLengthMs() const { return breakMin_ * minuteMs_; }
  uint32_t sessionLengthMs() const { return sessionMin_ * minuteMs_; }
  uint32_t idleSince() const { return idleSince_; }

 private:
  void startPhase(Mode m, uint32_t lengthMs, uint32_t now);
  void ask(Mode prompt, uint32_t now);
  void goIdle(uint32_t now);
  Alert startSession(uint8_t minutes, uint32_t now);

  const uint32_t minuteMs_, breakMin_, promptTimeoutMs_;
  Mode mode_ = Mode::Idle;
  Mode pausedPhase_ = Mode::Focus;
  uint8_t sessionMin_ = 30;    // last chosen session length (for "repeat")
  uint32_t phaseLen_ = 0;      // full length of the running phase
  uint32_t endAt_ = 0;         // millis() when the running phase ends
  uint32_t pausedLeft_ = 0;    // time left when paused
  uint32_t promptSince_ = 0;   // when the current prompt appeared
  uint32_t idleSince_ = 0;     // when we became idle
};
