#pragma once
#include <Arduino.h>
#include "AnalogButton.h"
#include "Config.h"

struct InputsSnapshot {
  int32_t encDelta = 0;     // +/- за цей тик
  bool okPressed = false;   // one-shot
  bool backPressed = false; // one-shot (GPIO back)
  bool a0BackPressed = false; // one-shot (Analog A0 back)
  bool encSwPressed = false;  // one-shot
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

  // analog back
  AnalogButton _a0Back;
};
