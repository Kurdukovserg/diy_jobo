#pragma once
#include <Arduino.h>

struct MotorConfig {
  int   stepsPerRev = 200;
  int   microsteps  = 16;

  float accelRpmPerSec   = 60.0f; // ramp speed
  bool  reverseEnabled   = true;
  float reverseEverySec  = 10.0f; // 0 => no reverse
};

class MotorController {
public:
  void begin(const MotorConfig& cfg);

  void setRun(bool run);
  void setTargetRpm(float rpm);     // 0..clamped
  void setReverseEnabled(bool en);
  void setReverseEverySec(float sec);

  void tick(); // call often

  float currentRpm() const { return _currentRpm; }
  bool  dirFwd() const { return _dirFwd; }

private:
  MotorConfig _cfg{};

  bool  _run = false;
  bool  _dirFwd = true;

  float _targetRpm  = 0.0f; // user target
  float _currentRpm = 0.0f; // ramped

  uint32_t _lastUs = 0;

  // soft reverse state machine
  enum RevState { RS_RUN, RS_RAMP_DOWN, RS_SWITCH_DIR, RS_RAMP_UP };
  RevState _rs = RS_RUN;
  float _savedTarget = 0.0f;
  uint32_t _lastReverseMs = 0;

  float rpmToSps(float rpm) const;
};
