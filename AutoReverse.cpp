#include "AutoReverse.h"
#include "Config.h"
#include <Arduino.h>
#include <math.h>

void AutoReverse::reset() {
  _lastMs = millis();
  _virtualTurns = 0.0f;
}

void AutoReverse::tick(float currentSps, bool running, void (*reverseFn)()) {
  if (!running) return;
  if (_cfg.mode == AutoRevMode::Off || _cfg.interval == 0) return;

  unsigned long now = millis();

  if (_cfg.mode == AutoRevMode::Time) {
    if (now - _lastMs >= (unsigned long)_cfg.interval * 1000UL) {
      _lastMs = now;
      reverseFn();
    }
    return;
  }

  if (_cfg.mode == AutoRevMode::Turns) {
    // Use real elapsed time for accurate turn counting
    float dtSec = (now - _lastMs) / 1000.0f;
    _lastMs = now;

    // turns = steps / steps_per_rev, steps = sps * dt
    _virtualTurns += fabs(currentSps) * dtSec / (float)STEPS_EFF;

    if (_virtualTurns >= _cfg.interval) {
      _virtualTurns = 0.0f;
      reverseFn();
    }
  }
}
