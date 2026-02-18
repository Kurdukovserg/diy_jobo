#include "MotorController.h"
#include "StepperISR.h"
#include <math.h>

void MotorController::begin(const MotorConfig& cfg) {
  _cfg = cfg;
  _lastUs = micros();
  _lastReverseMs = millis();
}

void MotorController::setRun(bool run) {
  _run = run;
  if (!run) {
    _targetRpm = 0.0f;
    _rs = RS_RUN;
  }
}

void MotorController::setTargetRpm(float rpm) {
  if (rpm < 0) rpm = 0;
  // clamp - для JOBO більше 80 зазвичай не треба
  if (rpm > 120) rpm = 120;
  _targetRpm = rpm;
}

void MotorController::setReverseEnabled(bool en) {
  _cfg.reverseEnabled = en;
}

void MotorController::setReverseEverySec(float sec) {
  if (sec < 0) sec = 0;
  _cfg.reverseEverySec = sec;
}

float MotorController::rpmToSps(float rpm) const {
  float stepsEff = (float)_cfg.stepsPerRev * (float)_cfg.microsteps;
  return rpm * stepsEff / 60.0f;
}

void MotorController::tick() {
  uint32_t nowUs = micros();
  float dt = (nowUs - _lastUs) / 1000000.0f;
  _lastUs = nowUs;
  if (dt <= 0) return;

  uint32_t nowMs = millis();

  // reverse trigger
  bool reverseDue =
      _run &&
      _cfg.reverseEnabled &&
      _cfg.reverseEverySec > 0.1f &&
      (nowMs - _lastReverseMs) >= (uint32_t)(_cfg.reverseEverySec * 1000.0f);

  if (reverseDue && _rs == RS_RUN) {
    _lastReverseMs = nowMs;
    _savedTarget = _targetRpm;
    _rs = RS_RAMP_DOWN;
  }

  float effectiveTarget = 0.0f;

  if (!_run) {
    effectiveTarget = 0.0f;
    _rs = RS_RUN;
  } else {
    switch (_rs) {
      case RS_RUN:        effectiveTarget = _targetRpm;   break;
      case RS_RAMP_DOWN:  effectiveTarget = 0.0f;         break;
      case RS_SWITCH_DIR: effectiveTarget = 0.0f;         break;
      case RS_RAMP_UP:    effectiveTarget = _savedTarget; break;
    }
  }

  // time-based ramp
  float maxDelta = _cfg.accelRpmPerSec * dt;
  float diff = effectiveTarget - _currentRpm;

  if (fabs(diff) <= maxDelta) _currentRpm = effectiveTarget;
  else _currentRpm += (diff > 0 ? maxDelta : -maxDelta);

  // reverse transitions at zero
  if (_run) {
    if (_rs == RS_RAMP_DOWN && _currentRpm <= 0.01f) {
      _rs = RS_SWITCH_DIR;
    }
    if (_rs == RS_SWITCH_DIR) {
      _dirFwd = !_dirFwd;
      _rs = RS_RAMP_UP;
    }
    if (_rs == RS_RAMP_UP && fabs(_currentRpm - _savedTarget) <= 0.01f) {
      _rs = RS_RUN;
    }
  }

  // push to stepper
  if (!_run || _currentRpm <= 0.01f) {
    stepperISR.stop();
    return;
  }

  float sps = rpmToSps(_currentRpm);
  if (!_dirFwd) sps = -sps;

  stepperISR.setSpeedSps(sps);
}
