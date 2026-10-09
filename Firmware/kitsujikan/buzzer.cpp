#include "buzzer.h"

void Buzzer::begin(uint8_t pin) {
  pin_ = pin;
  pinMode(pin_, OUTPUT);
  digitalWrite(pin_, LOW);  // transistor off = silent
}

void Buzzer::play(const Note* melody, uint8_t length) {
  melody_ = melody;
  length_ = length;
  index_ = 0;
  startNote(millis());
}

void Buzzer::stop() {
  noTone(pin_);
  digitalWrite(pin_, LOW);
  melody_ = nullptr;
}

void Buzzer::startNote(uint32_t now) {
  noteStart_ = now;
  const uint16_t f = melody_[index_].freq;
  if (f) tone(pin_, f);
  else   noTone(pin_);
}

void Buzzer::update(uint32_t now) {
  if (!melody_) return;
  if (now - noteStart_ < melody_[index_].ms) return;
  if (++index_ >= length_) { stop(); return; }
  startNote(now);
}

namespace Sounds {
// Notes: C6=1047, E6=1319, G6=1568, C7=2093. Passive buzzers are loudest around 2-4 kHz.
const Note CLICK[]      = {{2600, 12}};
const Note START[]      = {{1568, 60}, {0, 30}, {2093, 90}};
const Note FOCUS_DONE[] = {{1047, 120}, {1319, 120}, {1568, 120}, {2093, 250}, {0, 200},
                           {1047, 120}, {1319, 120}, {1568, 120}, {2093, 250}};
const Note BREAK_DONE[] = {{2093, 150}, {0, 100}, {2093, 150}, {0, 100}, {2093, 300}};
const Note STOP[]       = {{1568, 80}, {0, 30}, {1047, 140}};
const uint8_t CLICK_LEN      = sizeof(CLICK) / sizeof(Note);
const uint8_t START_LEN      = sizeof(START) / sizeof(Note);
const uint8_t FOCUS_DONE_LEN = sizeof(FOCUS_DONE) / sizeof(Note);
const uint8_t BREAK_DONE_LEN = sizeof(BREAK_DONE) / sizeof(Note);
const uint8_t STOP_LEN       = sizeof(STOP) / sizeof(Note);
}  // namespace Sounds
