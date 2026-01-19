#pragma once
#include <Arduino.h>

class EncoderInput {
public:
  void begin(uint8_t encA, uint8_t encB, uint8_t btnOk, uint8_t btnBack);
  void tick();

  bool rpmChanged() const { return _rpmDelta != 0; }
  int  consumeRpmDelta();

  bool okPressed();    // OK button pressed (start/stop toggle)
  bool backPressed();  // BACK button pressed (stop/menu)

private:
  uint8_t _encA = 0, _encB = 0;
  uint8_t _btnOk = 0, _btnBack = 0;

  int _lastEncA = HIGH;
  uint32_t _encLastUs = 0;

  bool _lastOk = true;
  bool _lastBack = true;

  int _rpmDelta = 0;
  bool _okFlag = false;
  bool _backFlag = false;
};
