#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class TempSensor {
public:
  void begin(uint8_t pin);
  void tick();

  bool hasReading() const { return _hasReading; }
  float tempC() const { return _tempC; }

private:
  OneWire* _oneWire = nullptr;
  DallasTemperature* _sensors = nullptr;
  DeviceAddress _addr;

  bool _found = false;
  bool _hasReading = false;
  float _tempC = NAN;

  enum Phase { Idle, Requested };
  Phase _phase = Idle;
  uint32_t _phaseTs = 0;
  uint32_t _lastCycleMs = 0;
};
