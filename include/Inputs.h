#pragma once
#include <Arduino.h>
#include "AnalogButton.h"
#include "Config.h"

struct InputsSnapshot {
  int32_t encDelta = 0;

  // one-shot events
  bool okPressed = false;
  bool backPressed = false;
  bool a0BackPressed = false;
  bool encSwPressed = false;
  bool encSwLongPress = false;  // long press on encoder switch

  // live states (for UI debug)
  bool okDown = false;
  bool backDown = false;
  bool a0BackDown = false;
  bool encSwDown = false;
  bool encSwRawHigh = true; // raw digitalRead == HIGH?
};


class Inputs {
public:
  void begin();
  InputsSnapshot tick(); // викликаєш у loop(), повертає події за цей кадр

private:
  // encoder
  int _lastEncA = HIGH;
  uint32_t _encLastUs = 0;
  static constexpr uint32_t ENC_DEBOUNCE_US = 1200;

  // gpio buttons last state
  bool _lastOk = HIGH;
  bool _lastBack = HIGH;
  bool _lastEncSw = HIGH;
  
  // long press detection
  uint32_t _encSwDownMs = 0;
  bool _encSwLongFired = false;
  static constexpr uint32_t LONG_PRESS_MS = 800;
  
  // startup delay to ignore false button presses
  uint32_t _startupMs = 0;
  static constexpr uint32_t STARTUP_IGNORE_MS = 500;

  // analog back
  AnalogButton _a0Back;
};
