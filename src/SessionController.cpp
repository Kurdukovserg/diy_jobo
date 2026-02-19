#include "SessionController.h"

void SessionController::begin(MotorController* motor) {
  _motor = motor;
  _running = false;
  _paused = false;
  _currentStep = 0;
  if (_motor) {
    _motor->setRun(false);
  }
}

void SessionController::tick() {
  if (!_motor) return;
  
  checkTempLimits();
  updateTimer();
  applyToMotor();
  _motor->tick();
}

void SessionController::checkTempLimits() {
  if (!_settings.tempLimitsEnabled || isnan(_currentTempC)) {
    _tempAlarm = false;
    _tempLow = false;
    _tempHigh = false;
    return;
  }
  
  _tempLow = _currentTempC < _settings.tempMin;
  _tempHigh = _currentTempC > _settings.tempMax;
  bool wasAlarm = _tempAlarm;
  _tempAlarm = _tempLow || _tempHigh;
  
  // Apply alarm action when alarm starts (not on every tick)
  if (_tempAlarm && !wasAlarm && _running) {
    // Get alarm action (step override or profile)
    TempAlarmAction action = _settings.tempAlarmAction;
    if (_currentStep >= 0 && _currentStep < _settings.stepCount) {
      const auto& step = _settings.steps[_currentStep];
      if (step.tempCoefOverride) {
        action = step.tempAlarmAction;
      }
    }
    
    switch (action) {
      case TempAlarmAction::None:
        // Just display, no action
        break;
      case TempAlarmAction::Beep:
        // Beep is handled by App.cpp via buzzer alerts
        break;
      case TempAlarmAction::Pause:
        // Pause the process
        _running = false;
        _paused = true;
        _stepPausedMs += millis() - _stepStartMs;
        _timerActive = false;
        Serial.println("TempAlarm: PAUSE action triggered");
        break;
      case TempAlarmAction::Stop:
        // Stop the process completely
        stop();
        Serial.println("TempAlarm: STOP action triggered");
        break;
    }
  }
}

void SessionController::start() {
  Serial.printf("start(): paused=%d\n", _paused);
  if (_paused) {
    // Resume from pause between steps
    _paused = false;
    _running = true;
    _timerActive = true;
    _stepStartMs = millis();
    Serial.println("  -> resumed from pause");
  } else {
    // Fresh start
    _running = true;
    _currentStep = 0;
    int32_t stepDur = _settings.steps[_currentStep].durationSec;
    if (stepDur > 0) {
      _timerActive = true;
      _stepStartMs = millis();
      _stepPausedMs = 0;
    }
    Serial.printf("  -> fresh start, stepDur=%d\n", stepDur);
  }
}

void SessionController::stop() {
  Serial.println("stop() called");
  _running = false;
  _paused = false;
  resetTimer();
}

void SessionController::toggleRun() {
  Serial.printf("toggleRun: run=%d pause=%d step=%d\n", _running, _paused, _currentStep);
  if (_running) {
    // Pause current step
    if (_timerActive) {
      _stepPausedMs += millis() - _stepStartMs;
      _timerActive = false;
    }
    _running = false;
    Serial.println("  -> paused mid-step");
  } else if (_paused) {
    // Resume after step-end pause (go to next step)
    Serial.println("  -> calling nextStep");
    nextStep();
  } else {
    // Start/resume
    _running = true;
    int32_t stepDur = _settings.steps[_currentStep].durationSec;
    if (stepDur > 0 && !_timerActive) {
      _timerActive = true;
      _stepStartMs = millis();
    }
    Serial.printf("  -> started, stepDur=%d\n", stepDur);
  }
}

void SessionController::nextStep() {
  Serial.printf("nextStep: cur=%d stepCount=%d\n", _currentStep, _settings.stepCount);
  if (_currentStep + 1 < _settings.stepCount) {
    _currentStep++;
    _paused = false;
    _running = true;
    _stepPausedMs = 0;
    int32_t stepDur = _settings.steps[_currentStep].durationSec;
    Serial.printf("  -> step %d dur=%d\n", _currentStep, stepDur);
    if (stepDur > 0) {
      _timerActive = true;
      _stepStartMs = millis();
    }
  } else {
    Serial.println("  -> all done, stop()");
    stop();
  }
}

void SessionController::applyToMotor() {
  if (!_motor) return;
  
  bool shouldRun = _running && !_paused;
  float rpm = adjustedRpm();
  
  // Debug only on state change
  static bool lastShouldRun = false;
  if (shouldRun != lastShouldRun) {
    Serial.printf("applyToMotor: run=%d pause=%d -> motor=%d rpm=%.1f\n", 
                  _running, _paused, shouldRun, rpm);
    lastShouldRun = shouldRun;
  }
  
  _motor->setRun(shouldRun);
  _motor->setTargetRpm(rpm);
  _motor->setReverseEnabled(_settings.reverseEnabled);
  _motor->setReverseEverySec(_settings.reverseIntervalSec);
}

float SessionController::calcTempCoefMultiplier() const {
  return calcTempCoefMultiplierForStep(_currentStep);
}

float SessionController::calcTempCoefMultiplierForStep(int8_t stepIdx) const {
  if (isnan(_currentTempC)) return 1.0f;
  
  // Check if step has override
  bool enabled = _settings.tempCoefEnabled;
  float base = _settings.tempCoefBase;
  float percent = _settings.tempCoefPercent;
  
  if (stepIdx >= 0 && stepIdx < _settings.stepCount) {
    const auto& step = _settings.steps[stepIdx];
    if (step.tempCoefOverride) {
      enabled = step.tempCoefEnabled;
      base = step.tempCoefBase;
      percent = step.tempCoefPercent;
    }
  }
  
  if (!enabled) return 1.0f;
  
  float tempDiff = _currentTempC - base;
  return 1.0f + (tempDiff * percent / 100.0f);
}

float SessionController::adjustedRpm() const {
  // Use current step's RPM if running
  float rpm = (float)_settings.targetRpm;
  if (_currentStep >= 0 && _currentStep < _settings.stepCount && _running) {
    rpm = (float)_settings.steps[_currentStep].rpm;
  }
  
  // Get temp coef settings (step override or profile)
  bool enabled = _settings.tempCoefEnabled;
  TempCoefTarget target = _settings.tempCoefTarget;
  if (_currentStep >= 0 && _currentStep < _settings.stepCount) {
    const auto& step = _settings.steps[_currentStep];
    if (step.tempCoefOverride) {
      enabled = step.tempCoefEnabled;
      target = step.tempCoefTarget;
    }
  }
  
  if (enabled && !isnan(_currentTempC)) {
    if (target == TempCoefTarget::Rpm || target == TempCoefTarget::Both) {
      rpm *= calcTempCoefMultiplier();
      if (rpm < 1.0f) rpm = 1.0f;
      if (rpm > 80.0f) rpm = 80.0f;
    }
  }
  return rpm;
}

int32_t SessionController::adjustedStepDurationSec(int8_t stepIdx) const {
  if (stepIdx < 0 || stepIdx >= _settings.stepCount) return 0;
  const auto& step = _settings.steps[stepIdx];
  int32_t dur = step.durationSec;
  if (dur == 0) return 0;
  
  // Get temp coef settings (step override or profile)
  bool enabled = _settings.tempCoefEnabled;
  TempCoefTarget target = _settings.tempCoefTarget;
  if (step.tempCoefOverride) {
    enabled = step.tempCoefEnabled;
    target = step.tempCoefTarget;
  }
  
  if (enabled && !isnan(_currentTempC)) {
    if (target == TempCoefTarget::Timer || target == TempCoefTarget::Both) {
      float mult = calcTempCoefMultiplierForStep(stepIdx);
      // For timer: higher temp = shorter time (inverse relationship)
      // Standard film dev: each degree above base reduces time
      dur = (int32_t)(dur / mult);
      if (dur < 1) dur = 1;
    }
  }
  return dur;
}

int32_t SessionController::stepRemainingSec() const {
  if (_currentStep >= _settings.stepCount) return 0;
  int32_t stepDur = adjustedStepDurationSec(_currentStep);
  if (stepDur == 0) return 0;
  
  uint32_t elapsedMs;
  if (_timerActive) {
    elapsedMs = millis() - _stepStartMs + _stepPausedMs;
  } else if (_stepPausedMs > 0) {
    elapsedMs = _stepPausedMs;
  } else {
    return stepDur;
  }
  
  int32_t remaining = stepDur - (int32_t)(elapsedMs / 1000);
  return remaining > 0 ? remaining : 0;
}

int32_t SessionController::totalRemainingSec() const {
  int32_t total = stepRemainingSec();
  for (int8_t i = _currentStep + 1; i < _settings.stepCount; i++) {
    total += adjustedStepDurationSec(i);
  }
  return total;
}

void SessionController::updateTimer() {
  if (_currentStep >= _settings.stepCount) return;
  if (!_running || _paused) return;
  
  int32_t stepDur = adjustedStepDurationSec(_currentStep);
  if (stepDur == 0) return;
  
  if (!_timerActive) {
    _timerActive = true;
    _stepStartMs = millis();
    Serial.printf("updateTimer: started timer for step %d, dur=%d\n", _currentStep, stepDur);
  }
  
  uint32_t elapsedMs = millis() - _stepStartMs + _stepPausedMs;
  if ((int32_t)(elapsedMs / 1000) >= stepDur) {
    // Step complete
    Serial.printf("updateTimer: step %d complete, elapsed=%lu\n", _currentStep, elapsedMs);
    _timerActive = false;
    _stepPausedMs = 0;
    
    if (_currentStep + 1 < _settings.stepCount) {
      // More steps - pause and wait for user
      Serial.println("  -> pausing for next step");
      _running = false;
      _paused = true;
    } else {
      // All done
      Serial.println("  -> all steps done");
      stop();
    }
  }
}

void SessionController::resetTimer() {
  _timerActive = false;
  _currentStep = 0;
  _stepPausedMs = 0;
}
