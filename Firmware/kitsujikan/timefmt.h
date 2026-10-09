// timefmt.h: turns milliseconds into "MM:SS" or "HH:MM:SS".
// Hours are shown only when non-zero; minutes and seconds always use 2 digits.
// Seconds are rounded UP, so a fresh 30-min session shows 30:00 (not 29:59)
// and the display reaches 00:00 exactly when the time is up.
#pragma once
#include <stdint.h>
#include <stdio.h>

inline void formatClock(uint32_t ms, char* out, size_t outSize) {
  const uint32_t totalSec = (ms + 999) / 1000;
  const unsigned h = totalSec / 3600;
  const unsigned m = (totalSec / 60) % 60;
  const unsigned s = totalSec % 60;
  if (h > 0) snprintf(out, outSize, "%02u:%02u:%02u", h, m, s);
  else       snprintf(out, outSize, "%02u:%02u", m, s);
}
