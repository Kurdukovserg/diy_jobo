#include "TempSensor.h"

void TempSensor::begin(uint8_t pin) {
  static OneWire ow(pin);
  static DallasTemperature dt(&ow);

  _ow = &ow;
  _dt = &dt;

  _dt->begin();
  _dt->setWaitForConversion(false);

  _hasSensor = _dt->getAddress(_addr, 0);
  if (_hasSensor) {
    _dt->setResolution(_addr, 12);
  }

  _lastCycleMs = millis();
  _phase = IDLE;
}

void TempSensor::tick() {
  // retry detect occasionally if missing
  if (!_hasSensor) {
    if (millis() - _lastCycleMs > 2000) {
      _lastCycleMs = millis();
      _hasSensor = _dt->getAddress(_addr, 0);
      if (_hasSensor) _dt->setResolution(_addr, 12);
    }
    return;
  }

  uint32_t now = millis();

  if (_phase == IDLE) {
    if (now - _lastCycleMs >= PERIOD_MS) {
      _lastCycleMs = now;
      _dt->requestTemperaturesByAddress(_addr);
      _phase = WAIT;
      _phaseTsMs = now;
    }
    return;
  }

  // WAIT
  if (now - _phaseTsMs >= CONV_WAIT_MS) {
    float t = _dt->getTempC(_addr);
    if (t > -80 && t < 150) {
      _tempC = t;
    } else {
      _tempC = NAN;
      _hasSensor = false; // force re-detect
    }
    _phase = IDLE;
  }
}
