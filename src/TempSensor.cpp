#include "TempSensor.h"
#include "Config.h"

void TempSensor::begin(uint8_t pin) {
  _oneWire = new OneWire(pin);
  _sensors = new DallasTemperature(_oneWire);

  _sensors->begin();
  _sensors->setWaitForConversion(false);

  _found = _sensors->getAddress(_addr, 0);
  if (_found) {
    _sensors->setResolution(_addr, 12);
  }

  _lastCycleMs = 0;  // trigger first read immediately
  _phase = Idle;
}

void TempSensor::tick() {
  if (!_found) {
    if (millis() - _lastCycleMs > 2000) {
      _lastCycleMs = millis();
      _found = _sensors->getAddress(_addr, 0);
      if (_found) {
        _sensors->setResolution(_addr, 12);
      }
    }
    return;
  }

  uint32_t now = millis();

  if (_phase == Idle) {
    if (now - _lastCycleMs >= TEMP_PERIOD_MS) {
      _lastCycleMs = now;
      _sensors->requestTemperaturesByAddress(_addr);
      _phase = Requested;
      _phaseTs = now;
    }
    return;
  }

  if (_phase == Requested) {
    if (now - _phaseTs >= TEMP_CONV_MS) {
      float t = _sensors->getTempC(_addr);
      if (t > -100.0f && t < 150.0f) {
        _tempC = t;
        _hasReading = true;
      } else {
        _tempC = NAN;
        _hasReading = false;
        _found = false;
      }
      _phase = Idle;
    }
  }
}
