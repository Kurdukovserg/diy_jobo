#pragma once
#include "Types.h"

class AutoReverse {
public:
  void setConfig(AutoRevConfig cfg) { _cfg = cfg; }
  const AutoRevConfig& config() const { return _cfg; }

  void reset();
  void tick(float currentSps, bool running, void (*reverseFn)());

private:
  AutoRevConfig _cfg;
  unsigned long _lastMs = 0;
  float _virtualTurns = 0.0f;
};
