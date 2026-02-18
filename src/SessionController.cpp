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
  
  updateTimer();
  applyToMotor();
  _motor->tick();
}

void SessionController::start() {
  if (_paused) {
    // Resume from pause between steps
    _paused = false;
    _running = true;
    _timerActive = true;
    _stepStartMs = millis();
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
  }
}

void SessionController::stop() {
  _running = false;
  _paused = false;
  resetTimer();
}

void SessionController::toggleRun() {
  if (_running) {
    // Pause current step
    if (_timerActive) {
      _stepPausedMs += millis() - _stepStartMs;
      _timerActive = false;
    }
    _running = false;
  } else if (_paused) {
    // Resume after step-end pause (go to next step)
    nextStep();
  } else {
    // Start/resume
    _running = true;
    int32_t stepDur = _settings.steps[_currentStep].durationSec;
    if (stepDur > 0 && !_timerActive) {
      _timerActive = true;
      _stepStartMs = millis();
    }
  }
}

void SessionController::nextStep() {
  if (_currentStep + 1 < _settings.stepCount) {
    _currentStep++;
    _paused = false;
    _running = true;
    _stepPausedMs = 0;
    int32_t stepDur = _settings.steps[_currentStep].durationSec;
    if (stepDur > 0) {
      _timerActive = true;
      _stepStartMs = millis();
    }
  } else {
    // All steps complete
    stop();
  }
}

void SessionController::applyToMotor() {
  if (!_motor) return;
  
  // Motor runs when running and not paused between steps
  _motor->setRun(_running && !_paused);
  _motor->setTargetRpm((float)_settings.targetRpm);
  _motor->setReverseEnabled(_settings.reverseEnabled);
  _motor->setReverseEverySec(_settings.reverseIntervalSec);
}

int32_t SessionController::stepRemainingSec() const {
  if (_currentStep >= _settings.stepCount) return 0;
  int32_t stepDur = _settings.steps[_currentStep].durationSec;
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
    total += _settings.steps[i].durationSec;
  }
  return total;
}

void SessionController::updateTimer() {
  if (_currentStep >= _settings.stepCount) return;
  if (!_running || _paused) return;
  
  int32_t stepDur = _settings.steps[_currentStep].durationSec;
  if (stepDur == 0) return;
  
  if (!_timerActive) {
    _timerActive = true;
    _stepStartMs = millis();
  }
  
  uint32_t elapsedMs = millis() - _stepStartMs + _stepPausedMs;
  if ((int32_t)(elapsedMs / 1000) >= stepDur) {
    // Step complete
    _timerActive = false;
    _stepPausedMs = 0;
    
    if (_currentStep + 1 < _settings.stepCount) {
      // More steps - pause and wait for user
      _running = false;
      _paused = true;
    } else {
      // All done
      stop();
    }
  }
}

void SessionController::resetTimer() {
  _timerActive = false;
  _currentStep = 0;
  _stepPausedMs = 0;
}
