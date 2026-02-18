#include "Inputs.h"

void Inputs::begin() {
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  pinMode(PIN_ENC_SW, INPUT_PULLUP);

  pinMode(PIN_BTN_OK, INPUT_PULLUP);
  pinMode(PIN_BTN_BACK, INPUT_PULLUP);

  _lastEncA = digitalRead(PIN_ENC_A);
  _lastOk = digitalRead(PIN_BTN_OK);
  _lastBack = digitalRead(PIN_BTN_BACK);
  _lastEncSw = digitalRead(PIN_ENC_SW);

  AnalogButton::Config cfg;
  cfg.pressThreshold = 700;
  cfg.releaseThreshold = 300;
  cfg.debounceMs = 60;
  _a0Back.begin(cfg);
  
  _startupMs = millis();
}

InputsSnapshot Inputs::tick() {
  InputsSnapshot s{};
  
  // Ignore button presses during startup (analog settling)
  bool ignoreButtons = (millis() - _startupMs) < STARTUP_IGNORE_MS;

  // --- encoder (polling, falling edge only, debounced by time) ---
  int a = digitalRead(PIN_ENC_A);
  if (a != _lastEncA) {
    uint32_t now = micros();
    if (now - _encLastUs > ENC_DEBOUNCE_US) {
      if (a == LOW) {
        int b = digitalRead(PIN_ENC_B);
        // твоя логіка напрямку
        if (b != a) s.encDelta += 1;
        else        s.encDelta -= 1;
      }
      _encLastUs = now;
    }
    _lastEncA = a;
  }

  // --- OK button via A0 (pulled up, LOW = pressed) ---
  _a0Back.tick();
  s.okDown = _a0Back.isPressed();
  if (_a0Back.wasPressed() && !ignoreButtons) s.okPressed = true;

  bool back = digitalRead(PIN_BTN_BACK);
  s.backDown = (back == LOW);
  if (back == LOW && _lastBack == HIGH) s.backPressed = true;
  _lastBack = back;

bool sw = digitalRead(PIN_ENC_SW);
uint32_t nowMs = millis();

s.encSwRawHigh = (sw == HIGH);
s.encSwDown = (sw == LOW);

// Long press detection
if (sw == LOW && _lastEncSw == HIGH) {
  // Just pressed - start timing
  _encSwDownMs = nowMs;
  _encSwLongFired = false;
}
if (sw == LOW && !_encSwLongFired && (nowMs - _encSwDownMs >= LONG_PRESS_MS)) {
  // Long press threshold reached
  s.encSwLongPress = true;
  _encSwLongFired = true;
}
if (sw == HIGH && _lastEncSw == LOW) {
  // Released - short press only if long wasn't fired
  if (!_encSwLongFired) {
    s.encSwPressed = true;
  }
}
_lastEncSw = sw;

  // A0 now used for OK, clear old a0Back fields
  s.a0BackDown = false;
  s.a0BackPressed = false;

  return s;
}
