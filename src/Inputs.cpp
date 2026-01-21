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
}

InputsSnapshot Inputs::tick() {
  InputsSnapshot s{};

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

  // --- gpio buttons (one-shot on press) ---
  bool ok = digitalRead(PIN_BTN_OK);
  if (ok == LOW && _lastOk == HIGH) s.okPressed = true;
  _lastOk = ok;

  bool back = digitalRead(PIN_BTN_BACK);
  if (back == LOW && _lastBack == HIGH) s.backPressed = true;
  _lastBack = back;

  bool sw = digitalRead(PIN_ENC_SW);
  if (sw == LOW && _lastEncSw == HIGH) s.encSwPressed = true;
  _lastEncSw = sw;

  // --- analog A0 back ---
  _a0Back.tick();
  if (_a0Back.wasPressed()) s.a0BackPressed = true;

  return s;
}
