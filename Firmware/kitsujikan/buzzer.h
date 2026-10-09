// buzzer.h: plays short melodies on the passive buzzer without blocking.
//
// A passive buzzer has no oscillator inside: we have to feed it a square wave
// (tone()) at the pitch we want. play() only stores the melody; update()
// moves to the next note when it's time, so buttons and the screen keep working.
#pragma once
#include <Arduino.h>

struct Note {
  uint16_t freq;  // Hz, 0 = silence
  uint16_t ms;    // duration
};

class Buzzer {
 public:
  void begin(uint8_t pin);
  void play(const Note* melody, uint8_t length);
  void stop();
  void update(uint32_t now);
  bool busy() const { return melody_ != nullptr; }

 private:
  void startNote(uint32_t now);
  uint8_t pin_ = 0;
  const Note* melody_ = nullptr;
  uint8_t length_ = 0, index_ = 0;
  uint32_t noteStart_ = 0;
};

// Ready-made sounds (defined in buzzer.cpp)
namespace Sounds {
extern const Note CLICK[];      extern const uint8_t CLICK_LEN;
extern const Note START[];      extern const uint8_t START_LEN;
extern const Note FOCUS_DONE[]; extern const uint8_t FOCUS_DONE_LEN;
extern const Note BREAK_DONE[]; extern const uint8_t BREAK_DONE_LEN;
extern const Note STOP[];       extern const uint8_t STOP_LEN;
}  // namespace Sounds
