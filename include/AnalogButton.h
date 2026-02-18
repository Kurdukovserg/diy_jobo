#pragma once
#include <Arduino.h>

class AnalogButton {
public:
  struct Config {
    uint16_t pressThreshold;
    uint16_t releaseThreshold;
    uint16_t debounceMs;

    // explicit defaults to avoid toolchain quirks
    Config()
      : pressThreshold(700),
        releaseThreshold(300),
        debounceMs(50) {}
  };

  void begin();                     // use defaults
  void begin(const Config& cfg);    // custom config
  void tick();

  bool isPressed() const { return _pressed; }
  bool wasPressed();    // one-shot
  bool wasReleased();   // one-shot

private:
  Config _cfg{};
  bool _pressed = false;

  bool _edgePressed = false;
  bool _edgeReleased = false;

  uint32_t _lastFlipMs = 0;
};
