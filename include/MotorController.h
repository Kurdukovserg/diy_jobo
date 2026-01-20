#pragma once
#include <Arduino.h>
#include "Types.h"
#include "StepperISR.h"

class MotorController {
public:
  MotorController() {}

  void begin(uint8_t stepPin, uint8_t dirPin, int rpm, bool dirFwd, RevParams rev, MotionParams motion);

  void setRunning(bool running) { _running = running; }
  bool running() const { return _running; }

  void setRpm(int rpm);
  void setDir(bool fwd);
  bool dirFwd() const { return _dirFwd; }

  void setRevParams(RevParams p);
  void setMotionParams(MotionParams p);

  void start(); // soft start
  void stop();  // soft stop
  void requestReverse(); // conditioned

  void tick();
  void run(); // stepper.runSpeed()

  float currentSps() const { return _currentSps; }
  bool isReversing() const { return _revPhase != RevPhase::None; }
  bool isStopped() const;
  bool shouldRun() const;

private:
  int _rpm = 10;
  bool _dirFwd = true;
  bool _running = false;

  RevParams _rev {350,150};
  MotionParams _motion {350,600};

  RevPhase  _revPhase  = RevPhase::None;
  RampPhase _rampPhase = RampPhase::None;

  unsigned long _phaseStartMs = 0;

  float _targetSps = 0.0f;
  float _currentSps = 0.0f;
  float _rampStartSps = 0.0f;

  float stepsPerSecFromRpm(int rpm) const;
  float ease(float t) const;
  void recomputeTarget();
  void setSpeed(float spsSigned);

  void startRampUp();
  void startRampDown();
  void tickRamp();

  bool isActivelySpinning() const;
  void startSoftReverse();
  void tickSoftReverse();
};
