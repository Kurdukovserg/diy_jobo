#pragma once
#include <Arduino.h>
#include "Inputs.h"

class App {
public:
  void begin();
  void tick();

private:
  Inputs _in;

  // simple state
  bool _run = false;
  int32_t _rpm = 30;

  static constexpr int RPM_MIN = 1;
  static constexpr int RPM_MAX = 80;

  void handleInputs(const InputsSnapshot& s);
  void clampRpm();
};
