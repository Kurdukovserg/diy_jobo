#include "AnalogButton.h"

void AnalogButton::begin() {
  Config cfg;
  begin(cfg);
}

void AnalogButton::begin(const Config& cfg) {
  _cfg = cfg;
  _pressed = false;
  _rawLast = false;
  _edgePressed = _edgeReleased = _edgeLongPress = false;
  _debounceUntilMs = 0;
  _pressedAtMs = 0;
  _longFired = false;
}

void AnalogButton::tick() {
  _edgePressed = false;
  _edgeReleased = false;
  _edgeLongPress = false;

  uint16_t v = analogRead(A0);
  uint32_t now = millis();

  // Button pulls LOW when pressed (v < pressThreshold = pressed)
  // Hysteresis: once pressed, need v > releaseThreshold to release
  bool raw = _rawLast ? (v < _cfg.releaseThreshold)   // stay pressed while v < release
                      : (v < _cfg.pressThreshold);    // become pressed when v < press

  // Debounce: ignore changes within lockout period
  if (now < _debounceUntilMs) {
    _rawLast = raw;
    return;
  }

  // Detect edges
  if (raw && !_pressed) {
    // Press detected - fire immediately
    _pressed = true;
    _edgePressed = true;
    _pressedAtMs = now;
    _longFired = false;
    _debounceUntilMs = now + _cfg.debounceMs;
  } else if (!raw && _pressed) {
    // Release detected
    _pressed = false;
    _edgeReleased = true;
    _debounceUntilMs = now + _cfg.debounceMs;
  }

  // Long press detection while held
  if (_pressed && !_longFired && (now - _pressedAtMs >= _cfg.longPressMs)) {
    _edgeLongPress = true;
    _longFired = true;
  }

  _rawLast = raw;
}

bool AnalogButton::wasPressed() {
  if (_edgePressed) { _edgePressed = false; return true; }
  return false;
}

bool AnalogButton::wasReleased() {
  if (_edgeReleased) { _edgeReleased = false; return true; }
  return false;
}

bool AnalogButton::wasLongPress() {
  if (_edgeLongPress) { _edgeLongPress = false; return true; }
  return false;
}
