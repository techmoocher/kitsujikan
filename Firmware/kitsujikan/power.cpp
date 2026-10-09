#include "power.h"
#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include "config.h"

static const uint8_t WAKE_PINS[] = {PIN_BTN_30, PIN_BTN_60, PIN_BTN_90};

uint8_t powerWakePin() {
  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_GPIO) return NO_WAKE_PIN;
  const uint64_t status = esp_sleep_get_gpio_wakeup_status();
  for (uint8_t pin : WAKE_PINS) {
    if (status & (1ULL << pin)) return pin;
  }
  return NO_WAKE_PIN;
}

void powerDeepSleep() {
  // If a wake button were still held, we'd wake up instantly. Wait for release.
  for (uint8_t pin : WAKE_PINS) {
    while (digitalRead(pin) == LOW) delay(10);
  }

  uint64_t mask = 0;
  for (uint8_t pin : WAKE_PINS) {
    // Keep the pull-ups on so the pins sit HIGH while asleep.
    // (GPIO2 also has the 10k pull-up on the board.)
    gpio_pullup_en((gpio_num_t)pin);
    gpio_pulldown_dis((gpio_num_t)pin);
    mask |= 1ULL << pin;
  }
  esp_deep_sleep_enable_gpio_wakeup(mask, ESP_GPIO_WAKEUP_GPIO_LOW);  // wake when any goes LOW
  esp_deep_sleep_start();
}
