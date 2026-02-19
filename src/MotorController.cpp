#include "MotorController.h"
#include "StepperISR.h"
#include <math.h>

void MotorController::begin(const MotorConfig& cfg) {
  _cfg = cfg;
  _lastUs = micros();
  _lastReverseMs = millis();
}

void MotorController::setRun(bool run) {
  if (run != _run) {
    Serial.printf("Motor setRun(%d) target=%.1f cur=%.1f\n", run, _targetRpm, _currentRpm);
  }
  
  // Reset state when starting
  if (run && !_run) {
    _currentRpm = 0.0f;      // Start ramp from zero
    _lastUs = micros();      // Reset timing to avoid huge dt
    _lastReverseMs = millis(); // Reset reverse timer
    _dirFwd = true;          // Start in forward direction
    _rs = RS_RUN;            // Normal running state
    stepperISR.kickStart();  // Force timer to wake up quickly
    Serial.println("Motor: reset state for fresh start");
  }
  
  _run = run;
  
  if (!run) {
    _targetRpm = 0.0f;
    _rs = RS_RUN;
    stepperISR.stop();       // Immediately stop the ISR
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
  if (!_run) {
    static bool lastWasRunning = false;
    if (lastWasRunning) {
      Serial.println("Motor tick: stopping stepper");
      lastWasRunning = false;
    }
    stepperISR.stop();
    return;
  } else {
    static bool announced = false;
    if (!announced) {
      Serial.println("Motor tick: run=true, proceeding");
      announced = true;
    }
  }
  
  // Debug ramp progress + ISR watchdog
  static uint32_t lastDebugMs = 0;
  static uint32_t lastPulseCount = 0;
  static uint32_t lastIsrCount = 0;
  static uint32_t lastWatchdogIsrCount = 0;
  static uint32_t lastWatchdogMs = 0;
  
  uint32_t currentIsrs = stepperISR.isrCount;
  
  // Watchdog: if ISR hasn't fired in 200ms while we expect it to, kick it
  if (nowMs - lastWatchdogMs > 200) {
    if (stepperISR._intervalUs > 0 && stepperISR._intervalUs < 50000) {
      // We expect ISR to be firing rapidly
      if (currentIsrs == lastWatchdogIsrCount) {
        // ISR stalled! Kick it (only log once per second max)
        static uint32_t lastWatchdogLog = 0;
        if (nowMs - lastWatchdogLog > 1000) {
          Serial.println("ISR watchdog: stall detected, kicking");
          lastWatchdogLog = nowMs;
        }
        stepperISR.kickStart();
      }
    }
    lastWatchdogIsrCount = currentIsrs;
    lastWatchdogMs = nowMs;
  }
  
  if (nowMs - lastDebugMs > 500) {
    uint32_t pulses = stepperISR.pulseCount;
    Serial.printf("tick: rpm=%.1f isr=%u pulse=%u interval=%u\n", 
                  _currentRpm, currentIsrs - lastIsrCount, pulses - lastPulseCount,
                  stepperISR._intervalUs);
    lastPulseCount = pulses;
    lastIsrCount = currentIsrs;
    lastDebugMs = nowMs;
  }

  // Always call setSpeedSps when running - even with small RPM
  // spsToIntervalUs handles small values by returning 0, which is fine
  float sps = rpmToSps(_currentRpm);
  if (!_dirFwd) sps = -sps;

  stepperISR.setSpeedSps(sps);
}
