#pragma once
#include <Arduino.h>

class AnalogButton {
public:
  struct Config {
    uint16_t pressThreshold   = 700;  // > = pressed
    uint16_t releaseThreshold = 300;  // < = released
    uint16_t debounceMs       = 40;
  };

  void begin(const Config& cfg = Config{});
  void tick();   // викликати в loop()

  bool isPressed() const { return _stablePressed; }

  // edge events (true тільки 1 тик)
  bool wasPressed();
  bool wasReleased();

private:
  Config _cfg{};

  bool _stablePressed = false;
  bool _lastReported  = false;

  uint32_t _lastChangeMs = 0;
  bool _edgePressed  = false;
  bool _edgeReleased = false;
};
