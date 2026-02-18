#include "AnalogButton.h"

void AnalogButton::begin() {
  Config cfg;        // defaults
  begin(cfg);
}

void AnalogButton::begin(const Config& cfg) {
  _cfg = cfg;
  _pressed = false;
  _edgePressed = _edgeReleased = false;
  _lastFlipMs = millis();
}

void AnalogButton::tick() {
  _edgePressed = false;
  _edgeReleased = false;

  uint16_t v = analogRead(A0);

  // hysteresis
  bool rawPressed = _pressed ? (v > _cfg.releaseThreshold)
                             : (v >= _cfg.pressThreshold);

  uint32_t now = millis();
  if (rawPressed != _pressed) {
    if (now - _lastFlipMs >= _cfg.debounceMs) {
      _pressed = rawPressed;
      if (_pressed) _edgePressed = true;
      else _edgeReleased = true;
      _lastFlipMs = now;
    }
  } else {
    _lastFlipMs = now;
  }
}

bool AnalogButton::wasPressed() {
  if (_edgePressed) { _edgePressed = false; return true; }
  return false;
}

bool AnalogButton::wasReleased() {
  if (_edgeReleased) { _edgeReleased = false; return true; }
  return false;
}
