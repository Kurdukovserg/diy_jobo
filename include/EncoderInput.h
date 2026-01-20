#pragma once
#include <Arduino.h>

class EncoderInput {
public:
  void begin(uint8_t encA, uint8_t encB, uint8_t encSw, uint8_t btnOk, uint8_t btnBack);
  void tick();

  bool rpmChanged() const { return _rpmDelta != 0; }
  int  consumeRpmDelta();

  bool okPressed();      // OK button pressed (start/stop toggle)
  bool backPressed();    // BACK button pressed (stop/menu)
  bool confirmPressed(); // Encoder SW pressed (enter menu/confirm)

  // ISR handler - called from interrupt
  static void IRAM_ATTR encoderISR();

private:
  static EncoderInput* _instance;
  
  uint8_t _encA = 0, _encB = 0, _encSw = 0;
  uint8_t _btnOk = 0, _btnBack = 0;

  // Encoder state machine (quadrature)
  volatile int8_t _encState = 0;
  volatile int _rpmDelta = 0;

  // Button states
  bool _lastOk = true;
  bool _lastBack = true;
  bool _lastConfirm = true;
  uint32_t _lastBtnMs = 0;

  volatile bool _okFlag = false;
  volatile bool _backFlag = false;
  volatile bool _confirmFlag = false;
};
