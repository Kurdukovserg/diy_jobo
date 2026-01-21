#include "AnalogButton.h"

void AnalogButton::begin(const Config& cfg) {
  _cfg = cfg;
  _stablePressed = false;
  _lastReported  = false;
  _lastChangeMs  = millis();
}

void AnalogButton::tick() {
  _edgePressed  = false;
  _edgeReleased = false;

  uint16_t v = analogRead(A0);

  bool rawPressed =
      (_stablePressed)
          ? (v > _cfg.releaseThreshold)  // hysteresis
          : (v > _cfg.pressThreshold);

  uint32_t now = millis();

  if (rawPressed != _stablePressed) {
    if (now - _lastChangeMs >= _cfg.debounceMs) {
      _stablePressed = rawPressed;

      if (_stablePressed && !_lastReported) {
        _edgePressed = true;
        _lastReported = true;
      }
      else if (!_stablePressed && _lastReported) {
        _edgeReleased = true;
        _lastReported = false;
      }

      _lastChangeMs = now;
    }
  } else {
    _lastChangeMs = now;
  }
}

bool AnalogButton::wasPressed() {
  if (_edgePressed) {
    _edgePressed = false;
    return true;
  }
  return false;
}

bool AnalogButton::wasReleased() {
  if (_edgeReleased) {
    _edgeReleased = false;
    return true;
  }
  return false;
}
