#pragma once
#include <Arduino.h>

enum class AlertType : uint8_t {
  None = 0,
  StepFinished,   // One short beep
  ProcessEnded,   // Three long beeps
  TempWarning     // Fast intermittent beeping (repeating)
};

struct BuzzerSettings {
  bool enabled = true;           // Master enable
  bool onStepFinished = true;    // Beep when step ends
  bool onProcessEnded = true;    // Beep when process ends
  bool onTempWarning = true;     // Beep on temp alarm
  uint16_t freqHz = 2200;        // Tone frequency (800-4000)
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

  void setSettings(const BuzzerSettings& s) {
    _settings = s;
    _cfg.freqHz = s.freqHz;
  }
  
  void setActiveHigh(bool high) { _cfg.activeHigh = high; }
  bool activeHigh() const { return _cfg.activeHigh; }
  
  BuzzerSettings& settings() { return _settings; }
  const BuzzerSettings& settings() const { return _settings; }

  void alert(AlertType type) {
    if (_cfg.pin == 255 || !_settings.enabled) return;
    
    // Check if this event type is enabled
    if (type == AlertType::StepFinished && !_settings.onStepFinished) return;
    if (type == AlertType::ProcessEnded && !_settings.onProcessEnded) return;
    if (type == AlertType::TempWarning && !_settings.onTempWarning) return;
    
    _alertType = type;
    _alertStep = 0;
    _nextActionMs = millis();
  }
  
  void stopAlert() {
    _alertType = AlertType::None;
    off();
  }
  
  void testBeep() {
    if (_cfg.pin == 255) return;
    // Immediately start tone for instant feedback
    on();
    _alertType = AlertType::StepFinished;
    _alertStep = 1;  // Already on, next tick will turn off
    _nextActionMs = millis() + _cfg.shortMs;
  }

  void tick() {
    // Handle software PWM for passive buzzer on ESP8266
#if !defined(ESP32)
    if (_toneOn) {
      uint32_t now = micros();
      if ((int32_t)(now - _nextToggleUs) >= 0) {
        _pinHigh = !_pinHigh;
        digitalWrite(_cfg.pin, _pinHigh ? HIGH : LOW);
        _nextToggleUs = now + _halfPeriodUs;
      }
    }
#endif

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
    // Software PWM for ESP8266 - avoids Timer1 conflict with StepperISR
    _halfPeriodUs = 500000UL / _cfg.freqHz;  // half period in microseconds
    _nextToggleUs = micros();
    _pinHigh = true;
    _toneOn = true;
    digitalWrite(_cfg.pin, HIGH);
#endif
  }

  void off() {
#if defined(ESP32)
    digitalWrite(_cfg.pin, _cfg.activeHigh ? LOW : HIGH);
#else
    _toneOn = false;
    digitalWrite(_cfg.pin, _cfg.activeHigh ? LOW : HIGH);
#endif
  }

  Config _cfg;
  BuzzerSettings _settings;
  AlertType _alertType = AlertType::None;
  uint8_t _alertStep = 0;
  uint32_t _nextActionMs = 0;
  
  // Software PWM state for ESP8266
#if !defined(ESP32)
  bool _toneOn = false;
  bool _pinHigh = false;
  uint32_t _halfPeriodUs = 227;  // ~2200Hz default
  uint32_t _nextToggleUs = 0;
#endif
};
