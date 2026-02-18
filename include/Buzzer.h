#pragma once
#include <Arduino.h>

enum class AlertType : uint8_t {
  None = 0,
  StepFinished,   // One short beep
  ProcessEnded,   // Three long beeps
  TempWarning     // Fast intermittent beeping (repeating)
};

class Buzzer {
public:
  struct Config {
    uint8_t pin = 255;
    bool activeHigh = true;
    uint16_t freqHz = 2200;     // for passive buzzer
    uint16_t shortMs = 50;      // short beep duration
    uint16_t longMs = 300;      // long beep duration
    uint16_t gapMs = 100;       // gap between beeps
    uint16_t fastMs = 80;       // fast beep for warning
    uint16_t fastGapMs = 80;    // fast gap for warning
  };

  void begin(const Config& cfg) {
    _cfg = cfg;
    if (_cfg.pin == 255) return;
    pinMode(_cfg.pin, OUTPUT);
    off();
  }

  void alert(AlertType type) {
    if (_cfg.pin == 255) return;
    _alertType = type;
    _alertStep = 0;
    _nextActionMs = millis();
    
    if (type == AlertType::None) {
      off();
    }
  }
  
  void stopAlert() {
    _alertType = AlertType::None;
    off();
  }

  void tick() {
    if (_alertType == AlertType::None) return;
    
    uint32_t now = millis();
    if ((int32_t)(now - _nextActionMs) < 0) return;
    
    switch (_alertType) {
      case AlertType::StepFinished:
        // One short beep then done
        if (_alertStep == 0) {
          on();
          _nextActionMs = now + _cfg.shortMs;
          _alertStep = 1;
        } else {
          off();
          _alertType = AlertType::None;
        }
        break;
        
      case AlertType::ProcessEnded:
        // Three long beeps: on-off-on-off-on-off
        if (_alertStep < 6) {
          if (_alertStep % 2 == 0) {
            on();
            _nextActionMs = now + _cfg.longMs;
          } else {
            off();
            _nextActionMs = now + _cfg.gapMs;
          }
          _alertStep++;
        } else {
          off();
          _alertType = AlertType::None;
        }
        break;
        
      case AlertType::TempWarning:
        // Fast repeating beeps (continues until stopAlert)
        if (_alertStep % 2 == 0) {
          on();
          _nextActionMs = now + _cfg.fastMs;
        } else {
          off();
          _nextActionMs = now + _cfg.fastGapMs;
        }
        _alertStep++;
        break;
        
      default:
        _alertType = AlertType::None;
        break;
    }
  }
  
  bool isAlerting() const { return _alertType != AlertType::None; }
  AlertType currentAlert() const { return _alertType; }

private:
  void on() {
#if defined(ESP32)
    digitalWrite(_cfg.pin, _cfg.activeHigh ? HIGH : LOW);
#else
    tone(_cfg.pin, _cfg.freqHz);
#endif
  }

  void off() {
#if defined(ESP32)
    digitalWrite(_cfg.pin, _cfg.activeHigh ? LOW : HIGH);
#else
    noTone(_cfg.pin);
    digitalWrite(_cfg.pin, _cfg.activeHigh ? LOW : HIGH);
#endif
  }

  Config _cfg;
  AlertType _alertType = AlertType::None;
  uint8_t _alertStep = 0;
  uint32_t _nextActionMs = 0;
};
