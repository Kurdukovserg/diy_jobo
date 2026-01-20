#include "MotorController.h"
#include "Config.h"
#include <math.h>

void MotorController::begin(uint8_t stepPin, uint8_t dirPin, int rpm, bool dirFwd, RevParams rev, MotionParams motion) {
  stepperISR.begin(stepPin, dirPin);
  
  _rpm = rpm;
  _dirFwd = dirFwd;
  _rev = rev;
  _motion = motion;

  recomputeTarget();
  _currentSps = 0.0f;
  setSpeed(0.0f);

  _revPhase = RevPhase::None;
  _rampPhase = RampPhase::None;
  _running = false;
}

void MotorController::setRpm(int rpm) {
  _rpm = rpm;
  recomputeTarget();
  // Don't jump to new speed - let tick() smoothly transition via setSpeed()
  // The target is updated, motor will naturally adjust on next runSpeed() calls
}

void MotorController::setDir(bool fwd) {
  _dirFwd = fwd;
  recomputeTarget();

  if (_running && _revPhase == RevPhase::None && _rampPhase == RampPhase::None) {
    _currentSps = _targetSps;
    setSpeed(_currentSps);
  }
}

void MotorController::setRevParams(RevParams p) { _rev = p; }
void MotorController::setMotionParams(MotionParams p) { _motion = p; }

float MotorController::stepsPerSecFromRpm(int rpm) const {
  return (rpm * (float)STEPS_EFF) / 60.0f;
}

float MotorController::ease(float t) const {
  return t * t * (3.0f - 2.0f * t);
}

void MotorController::recomputeTarget() {
  float mag = stepsPerSecFromRpm(_rpm);
  _targetSps = _dirFwd ? mag : -mag;
}

void MotorController::setSpeed(float spsSigned) {
  stepperISR.setSpeed(spsSigned);
}

void MotorController::startRampUp() {
  _rampPhase = RampPhase::Up;
  _phaseStartMs = millis();
  _rampStartSps = 0.0f;
}

void MotorController::startRampDown() {
  _rampPhase = RampPhase::Down;
  _phaseStartMs = millis();
  _rampStartSps = _currentSps;
}

void MotorController::tickRamp() {
  if (_rampPhase == RampPhase::None) return;

  unsigned long now = millis();
  uint16_t dur = (_rampPhase == RampPhase::Up) ? _motion.startRampMs : _motion.stopRampMs;
  if (dur < 50) dur = 50;

  float t = (now - _phaseStartMs) / float(dur);

  if (t >= 1.0f) {
    if (_rampPhase == RampPhase::Up) {
      _currentSps = _targetSps;
      setSpeed(_currentSps);
    } else {
      _currentSps = 0.0f;
      setSpeed(0.0f);
    }
    _rampPhase = RampPhase::None;
    return;
  }

  t = constrain(t, 0.0f, 1.0f);
  float k = ease(t);

  if (_rampPhase == RampPhase::Up) {
    _currentSps = _targetSps * k;
  } else {
    _currentSps = _rampStartSps * (1.0f - k);
  }
  setSpeed(_currentSps);
}

bool MotorController::isActivelySpinning() const {
  return _running && fabsf(_currentSps) > 1.0f;
}

void MotorController::start() {
  recomputeTarget();
  _running = true;
  if (_revPhase == RevPhase::None) startRampUp();
}

void MotorController::stop() {
  // stop має пріоритет над reverse
  _revPhase = RevPhase::None;
  startRampDown();
  _running = false;
}

void MotorController::startSoftReverse() {
  if (_revPhase != RevPhase::None) return;
  _revPhase = RevPhase::RampDown;
  _phaseStartMs = millis();
  _rampStartSps = _currentSps;
}

void MotorController::requestReverse() {
  if (_revPhase != RevPhase::None || _rampPhase != RampPhase::None) return;

  if (!isActivelySpinning()) {
    _dirFwd = !_dirFwd;
    recomputeTarget();
    if (_running) {
      _currentSps = _targetSps;
      setSpeed(_currentSps);
    }
    return;
  }

  startSoftReverse();
}

void MotorController::tickSoftReverse() {
  if (_revPhase == RevPhase::None) return;

  unsigned long now = millis();

  if (_revPhase == RevPhase::RampDown) {
    float t = (now - _phaseStartMs) / float(_rev.rampMs);
    if (t >= 1.0f) {
      _currentSps = 0.0f;
      setSpeed(0.0f);
      _revPhase = RevPhase::Pause;
      _phaseStartMs = now;
      return;
    }
    t = constrain(t, 0.0f, 1.0f);
    float k = 1.0f - ease(t);
    _currentSps = _rampStartSps * k;
    setSpeed(_currentSps);
    return;
  }

  if (_revPhase == RevPhase::Pause) {
    if (now - _phaseStartMs >= _rev.pauseMs) {
      _dirFwd = !_dirFwd;
      recomputeTarget();
      _revPhase = RevPhase::RampUp;
      _phaseStartMs = now;
    }
    return;
  }

  if (_revPhase == RevPhase::RampUp) {
    float t = (now - _phaseStartMs) / float(_rev.rampMs);
    if (t >= 1.0f) {
      _currentSps = _targetSps;
      setSpeed(_currentSps);
      _revPhase = RevPhase::None;
      return;
    }
    t = constrain(t, 0.0f, 1.0f);
    float k = ease(t);
    _currentSps = _targetSps * k;
    setSpeed(_currentSps);
    return;
  }
}

void MotorController::tick() {
  tickRamp();
  tickSoftReverse();

  // When running steady (no ramp/reverse), smoothly adjust to target if RPM changed
  if (_running && _revPhase == RevPhase::None && _rampPhase == RampPhase::None) {
    if (fabsf(_currentSps - _targetSps) > 1.0f) {
      // Smooth transition: move 5% toward target each tick
      _currentSps += (_targetSps - _currentSps) * 0.05f;
      setSpeed(_currentSps);
    } else if (fabsf(_currentSps - _targetSps) > 0.1f) {
      _currentSps = _targetSps;
      setSpeed(_currentSps);
    }
  }

  if (!_running && _revPhase == RevPhase::None && _rampPhase == RampPhase::None) {
    if (fabsf(_currentSps) > 0.0f) {
      _currentSps = 0.0f;
      setSpeed(0.0f);
    }
  }
}

void MotorController::run() {
  // No-op: stepping is now handled by timer interrupt in StepperISR
}

bool MotorController::isStopped() const {
  return (fabsf(_currentSps) < 0.5f) &&
         (_rampPhase == RampPhase::None) &&
         (_revPhase == RevPhase::None);
}

bool MotorController::shouldRun() const {
  return (fabsf(_currentSps) > 0.5f) ||
         (_rampPhase != RampPhase::None) ||
         (_revPhase != RevPhase::None);
}
