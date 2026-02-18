#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class TempSensor {
public:
  void begin(uint8_t pin);
  void tick();

  bool ok() const { return _hasSensor && !isnan(_tempC); }
  bool hasSensor() const { return _hasSensor; }
  float tempC() const { return _tempC; }

private:
  OneWire* _ow = nullptr;
  DallasTemperature* _dt = nullptr;

  DeviceAddress _addr{};
  bool _hasSensor = false;

  float _tempC = NAN;

  enum Phase { IDLE, WAIT };
  Phase _phase = IDLE;

  uint32_t _lastCycleMs = 0;
  uint32_t _phaseTsMs = 0;

  static constexpr uint32_t PERIOD_MS = 1200;
  static constexpr uint32_t CONV_WAIT_MS = 800;
};
