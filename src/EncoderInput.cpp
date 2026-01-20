#include "EncoderInput.h"
#include "Config.h"

void EncoderInput::begin(uint8_t encA, uint8_t encB, uint8_t encSw, uint8_t btnOk, uint8_t btnBack) {
  _encA = encA;
  _encB = encB;
  _encSw = encSw;
  _btnOk = btnOk;
  _btnBack = btnBack;

  pinMode(_encA, INPUT_PULLUP);
  pinMode(_encB, INPUT_PULLUP);
  pinMode(_encSw, INPUT_PULLUP);
  pinMode(_btnOk, INPUT_PULLUP);
  pinMode(_btnBack, INPUT_PULLUP);

  _lastEncA = digitalRead(_encA);
  _lastOk = true;
  _lastBack = true;
  _lastConfirm = true;
}

int EncoderInput::consumeRpmDelta() {
  int d = _rpmDelta;
  _rpmDelta = 0;
  return d;
}

bool EncoderInput::okPressed() {
  bool v = _okFlag;
  _okFlag = false;
  return v;
}

bool EncoderInput::backPressed() {
  bool v = _backFlag;
  _backFlag = false;
  return v;
}

bool EncoderInput::confirmPressed() {
  bool v = _confirmFlag;
  _confirmFlag = false;
  return v;
}

void EncoderInput::tick() {
  // Encoder rotation with debounce
  int a = digitalRead(_encA);
  uint32_t now = micros();
  
  if (a != _lastEncA && (now - _encLastUs > ENC_DEBOUNCE_US)) {
    _encLastUs = now;
    _lastEncA = a;
    
    if (a == LOW) { // falling edge - read direction
      int b = digitalRead(_encB);
      if (b == HIGH) _rpmDelta++;
      else _rpmDelta--;
    }
  }

  // OK button (active LOW)
  bool ok = digitalRead(_btnOk);
  if (ok == LOW && _lastOk == HIGH) {
    _okFlag = true;
  }
  _lastOk = ok;

  // BACK button (active LOW)
  bool back = digitalRead(_btnBack);
  if (back == LOW && _lastBack == HIGH) {
    _backFlag = true;
  }
  _lastBack = back;

  // Encoder SW / Confirm button (active LOW)
  bool confirm = digitalRead(_encSw);
  if (confirm == LOW && _lastConfirm == HIGH) {
    _confirmFlag = true;
  }
  _lastConfirm = confirm;
}
