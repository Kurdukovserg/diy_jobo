#pragma once
#include <Arduino.h>

class Buzzer {
public:
  struct Config {
    uint8_t pin = 255;
    bool activeHigh = true;
    uint16_t freqHz = 2200;     // for passive buzzer
    uint16_t clickMs = 25;      // short beep
    uint16_t gapMs = 40;        // gap between double beeps
  };

  void begin(const Config& cfg) {
    _cfg = cfg;
    if (_cfg.pin == 255) return;
    pinMode(_cfg.pin, OUTPUT);
    off();
  }

  void tick() {
    if (!_armed) return;
    const uint32_t now = millis();
    if (now >= _untilMs) {
      off();
      _armed = false;
    }
  }

  void click() { beep(_cfg.clickMs); }

  void doubleClick() {
    beep(_cfg.clickMs);
    _queue2 = true;
    _queue2At = millis() + _cfg.clickMs + _cfg.gapMs;
  }

  void tickQueued() {
    tick();
    if (_queue2 && (int32_t)(millis() - _queue2At) >= 0) {
      _queue2 = false;
      beep(_cfg.clickMs);
    }
  }

private:
  void beep(uint16_t ms) {
    if (_cfg.pin == 255) return;
    on();
    _armed = true;
    _untilMs = millis() + ms;
  }

  void on() {
#if defined(ESP32)
    digitalWrite(_cfg.pin, _cfg.activeHigh ? HIGH : LOW);
#else
    tone(_cfg.pin, _cfg.freqHz);
#endif
  }

  void off() {
#if defined(ESP32)
    digitalWrite(_cfg.pin, _cfg.activeHigh ? LOW : HIGH);
#else
    noTone(_cfg.pin);
    digitalWrite(_cfg.pin, _cfg.activeHigh ? LOW : HIGH);
#endif
  }

  Config _cfg;
  bool _armed = false;
  uint32_t _untilMs = 0;

  bool _queue2 = false;
  uint32_t _queue2At = 0;
};
