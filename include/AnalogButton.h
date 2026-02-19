#pragma once
#include <Arduino.h>

class AnalogButton {
public:
  struct Config {
    uint16_t pressThreshold;
    uint16_t releaseThreshold;
    uint16_t debounceMs;
    uint16_t longPressMs;

    Config()
      : pressThreshold(300),    // v < 300 = pressed
        releaseThreshold(700),  // v > 700 = released
        debounceMs(30),
        longPressMs(600) {}
  };

  void begin();
  void begin(const Config& cfg);
  void tick();

  bool isPressed() const { return _pressed; }
  bool wasPressed();      // one-shot, fires immediately on press
  bool wasReleased();     // one-shot
  bool wasLongPress();    // one-shot, fires when held longPressMs

private:
  Config _cfg{};
  bool _pressed = false;
  bool _rawLast = false;

  bool _edgePressed = false;
  bool _edgeReleased = false;
  bool _edgeLongPress = false;

  uint32_t _debounceUntilMs = 0;
  uint32_t _pressedAtMs = 0;
  bool _longFired = false;
};
