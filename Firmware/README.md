# Kitsujikan firmware

Arduino (C++) firmware for the Seeed XIAO ESP32-C3. It's compiled to native code, and because the Arduino ESP32 core is built on Espressif's ESP-IDF, the low-level APIs (deep sleep, calibrated ADC) are available directly.

## Build and upload (Arduino IDE 2)

1. **File → Preferences → Additional boards manager URLs:**
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
2. **Tools → Board → Boards Manager:** install **esp32 by Espressif Systems** (tested with 3.3.2).
3. **Sketch → Include Library → Manage Libraries:** install **Adafruit SSD1306**. Accept "Install all" so Adafruit GFX and BusIO come with it.
4. Open `Firmware/kitsujikan/kitsujikan.ino`.
5. **Tools → Board → esp32 → XIAO_ESP32C3**, then pick the port.
6. Keep **Tools → USB CDC On Boot: Enabled** (the default). `Serial` then goes over USB. If it were disabled, `Serial` would use the UART pins GPIO20/21, which this board uses for the buzzer and the red button.
7. Click **Upload**, and open the Serial Monitor at 115200 baud to see log messages.

If the upload doesn't start, hold **BOOT** on the XIAO, tap **RESET**, release BOOT, and try again.

## Testing tips

- Set `FAST_TEST` to `1` in `config.h`. Every "minute" then lasts one second, so a full cycle takes under two minutes.
- Test the timer logic on your laptop, no board needed (run from this `Firmware/` folder):
  ```
  g++ -std=c++17 -Wall -Ikitsujikan test/test_pomodoro.cpp kitsujikan/pomodoro.cpp -o test_pomodoro && ./test_pomodoro
  ```

## Controls

| State | 30 / 60 / 90 | Green | Red (tap) | Red (hold) |
|---|---|---|---|---|
| Idle | start that session | — | — | — |
| Focus / Break | ignored | — | pause | stop |
| Paused | ignored | resume | resume | stop |
| "Take a break?" | start that session instead | start 5-min break | skip break | — |
| "Again?" | start that session instead | repeat last session | back to idle | — |

After 1 minute idle, it shows "zzz" and goes into deep sleep. Pressing 30, 60 or 90 wakes it and starts that session right away.

## Files

| File | What it does |
|---|---|
| `config.h` | Pins and timings: change things here first |
| `pomodoro.*` | Timer state machine (pure C++, unit-tested) |
| `timefmt.h` | `MM:SS` / `HH:MM:SS` formatting |
| `button.*` | Debounced buttons: press, short release, long press |
| `buzzer.*` | Non-blocking melodies on the passive buzzer |
| `battery.*` | Battery voltage and percentage from the divider |
| `ui.*` | OLED screens |
| `power.*` | Deep sleep and wake-up |

## To check on real hardware

- That the internal pull-ups on GPIO3/5 stay on in deep sleep, so the device doesn't wake up by itself. GPIO2 also has a 10k pull-up on the board.
- The battery percentage against a multimeter reading. Adjust the table in `battery.cpp` if needed.
- Buzzer pitch and volume. Passive buzzers are loudest around 2–4 kHz. The notes are in `buzzer.cpp`.
