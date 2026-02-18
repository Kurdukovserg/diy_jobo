#include "Buzzer.h"

void Buzzer::begin(uint8_t pin) {
  _pin = pin;
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}

void Buzzer::tick() {
  if (!_playing || !_enabled) return;
  advancePattern();
}

void Buzzer::play(BuzzerPattern pattern) {
  if (!_enabled && pattern != BuzzerPattern::Alarm) return;
  _pattern = pattern;
  _step = 0;
  _stepStartMs = millis();
  _playing = true;
  
  // Start first step
  switch (_pattern) {
    case BuzzerPattern::Click:
    case BuzzerPattern::StepDone:
    case BuzzerPattern::SessionDone:
    case BuzzerPattern::Alarm:
      setBuzzer(true);
      break;
    default:
      _playing = false;
      break;
  }
}

void Buzzer::stop() {
  _playing = false;
  _pattern = BuzzerPattern::None;
  setBuzzer(false);
}

void Buzzer::setBuzzer(bool on) {
  _buzzerOn = on;
  digitalWrite(_pin, on ? HIGH : LOW);
}

void Buzzer::advancePattern() {
  uint32_t elapsed = millis() - _stepStartMs;
  
  switch (_pattern) {
    case BuzzerPattern::Click:
      // Single 50ms beep
      if (_step == 0 && elapsed >= 50) {
        setBuzzer(false);
        _playing = false;
      }
      break;
      
    case BuzzerPattern::StepDone:
      // Two 100ms beeps with 100ms gap
      // Step 0: beep ON (100ms)
      // Step 1: beep OFF (100ms)
      // Step 2: beep ON (100ms)
      // Step 3: done
      if (_step == 0 && elapsed >= 100) {
        setBuzzer(false);
        _step = 1;
        _stepStartMs = millis();
      } else if (_step == 1 && elapsed >= 100) {
        setBuzzer(true);
        _step = 2;
        _stepStartMs = millis();
      } else if (_step == 2 && elapsed >= 100) {
        setBuzzer(false);
        _playing = false;
      }
      break;
      
    case BuzzerPattern::SessionDone:
      // Long beep (500ms) + pause (200ms) + 3 short beeps
      // Step 0: long beep ON (500ms)
      // Step 1: pause OFF (200ms)
      // Step 2-6: short beep pattern (100ms on, 100ms off) x3
      if (_step == 0 && elapsed >= 500) {
        setBuzzer(false);
        _step = 1;
        _stepStartMs = millis();
      } else if (_step == 1 && elapsed >= 200) {
        setBuzzer(true);
        _step = 2;
        _stepStartMs = millis();
      } else if (_step >= 2 && _step < 7) {
        if (elapsed >= 100) {
          bool on = (_step % 2 == 0);
          setBuzzer(!on);
          _step++;
          _stepStartMs = millis();
          if (_step >= 8) {
            setBuzzer(false);
            _playing = false;
          }
        }
      } else if (_step >= 7) {
        setBuzzer(false);
        _playing = false;
      }
      break;
      
    case BuzzerPattern::Alarm:
      // Repeating: 200ms on, 200ms off
      if (elapsed >= 200) {
        setBuzzer(!_buzzerOn);
        _stepStartMs = millis();
      }
      break;
      
    default:
      _playing = false;
      break;
  }
}
