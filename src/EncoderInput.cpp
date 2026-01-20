#include "EncoderInput.h"
#include "Config.h"

// Static instance for ISR access
EncoderInput* EncoderInput::_instance = nullptr;

// Quadrature state table - filters noise, only valid transitions count
// Index = (prevState << 2) | currState, value = direction (-1, 0, +1)
static const int8_t ENC_STATE_TABLE[16] PROGMEM = {
   0, -1,  1,  0,   // 00 -> 00, 01, 10, 11
   1,  0,  0, -1,   // 01 -> 00, 01, 10, 11
  -1,  0,  0,  1,   // 10 -> 00, 01, 10, 11
   0,  1, -1,  0    // 11 -> 00, 01, 10, 11
};

void IRAM_ATTR EncoderInput::encoderISR() {
  if (!_instance) return;
  
  // Read both pins atomically
  uint8_t a = digitalRead(_instance->_encA);
  uint8_t b = digitalRead(_instance->_encB);
  uint8_t currState = (a << 1) | b;
  
  // Combine with previous state to get table index
  uint8_t idx = (_instance->_encState << 2) | currState;
  int8_t delta = pgm_read_byte(&ENC_STATE_TABLE[idx]);
  
  _instance->_rpmDelta += delta;
  _instance->_encState = currState;
}

void EncoderInput::begin(uint8_t encA, uint8_t encB, uint8_t encSw, uint8_t btnOk, uint8_t btnBack) {
  _encA = encA;
  _encB = encB;
  _encSw = encSw;
  _btnOk = btnOk;
  _btnBack = btnBack;
  _instance = this;

  pinMode(_encA, INPUT_PULLUP);
  pinMode(_encB, INPUT_PULLUP);
  pinMode(_encSw, INPUT_PULLUP);
  pinMode(_btnOk, INPUT_PULLUP);
  pinMode(_btnBack, INPUT_PULLUP);

  // Initialize encoder state
  _encState = (digitalRead(_encA) << 1) | digitalRead(_encB);
  _rpmDelta = 0;
  
  // Attach interrupt on pin A only (safer for ESP8266, D3/GPIO0 is boot pin)
  attachInterrupt(digitalPinToInterrupt(_encA), encoderISR, CHANGE);

  _lastOk = true;
  _lastBack = true;
  _lastConfirm = true;
  _lastBtnMs = 0;
}

int EncoderInput::consumeRpmDelta() {
  noInterrupts();
  int d = _rpmDelta;
  _rpmDelta = 0;
  interrupts();
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
  // Buttons are polled (with debounce), encoder is interrupt-driven
  uint32_t now = millis();
  
  // Debounce buttons - 50ms minimum between presses
  if (now - _lastBtnMs < 50) return;

  // OK button (active LOW)
  bool ok = digitalRead(_btnOk);
  if (ok == LOW && _lastOk == HIGH) {
    _okFlag = true;
    _lastBtnMs = now;
  }
  _lastOk = ok;

  // BACK button (active LOW)
  bool back = digitalRead(_btnBack);
  if (back == LOW && _lastBack == HIGH) {
    _backFlag = true;
    _lastBtnMs = now;
  }
  _lastBack = back;

  // Encoder SW / Confirm button (active LOW)
  bool confirm = digitalRead(_encSw);
  if (confirm == LOW && _lastConfirm == HIGH) {
    _confirmFlag = true;
    _lastBtnMs = now;
  }
  _lastConfirm = confirm;
}
