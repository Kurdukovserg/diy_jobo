#pragma once
// Mock MotorController for native testing

#include "ArduinoMock.h"

struct MotorConfig {
  int stepsPerRev = 200;
  int microsteps = 16;
  float accelRpmPerSec = 60.0f;
  bool reverseEnabled = true;
  float reverseEverySec = 10.0f;
};

class MotorController {
public:
  void begin(const MotorConfig& cfg) {}
  void setRun(bool run) { _run = run; _setRunCalls++; }
  void setTargetRpm(float rpm) { _targetRpm = rpm; }
  void setReverseEnabled(bool en) { _reverseEnabled = en; }
  void setReverseEverySec(float sec) {}
  void tick() {}
  float currentRpm() const { return _targetRpm; }
  bool dirFwd() const { return true; }
  
  void setStepsPerRev(int steps) {}
  void setMicrosteps(int ms) {}
  int stepsPerRev() const { return 200; }
  int microsteps() const { return 16; }
  
  // Test inspection
  bool _run = false;
  float _targetRpm = 0;
  bool _reverseEnabled = false;
  int _setRunCalls = 0;
  
  void reset() {
    _run = false;
    _targetRpm = 0;
    _reverseEnabled = false;
    _setRunCalls = 0;
  }
};
