#pragma once
#include <Arduino.h>
#include "MotorController.h"

enum class TempCoefTarget : uint8_t {
  Timer = 0,
  Rpm = 1,
  Both = 2
};

static constexpr int8_t MAX_STEPS = 10;

struct ProcessStep {
  int32_t durationSec = 0;  // 0 = disabled/unused
  // Future: could add step name, RPM override, etc.
};

struct SessionSettings {
  int32_t targetRpm = 30;
  
  // Reverse
  bool reverseEnabled = true;
  float reverseIntervalSec = 10.0f;
  
  // Process steps (development profile)
  ProcessStep steps[MAX_STEPS];
  int8_t stepCount = 1;  // number of active steps (1-10)
  
  // Temperature coefficient
  bool tempCoefEnabled = false;
  float tempCoefBase = 20.0f;
  float tempCoefPercent = 10.0f;
  TempCoefTarget tempCoefTarget = TempCoefTarget::Timer;
  
  // Temperature limits (alarm if outside range)
  bool tempLimitsEnabled = false;
  float tempMin = 18.0f;
  float tempMax = 24.0f;
  
  // Helper to get total duration
  int32_t totalDurationSec() const {
    int32_t total = 0;
    for (int8_t i = 0; i < stepCount; i++) {
      total += steps[i].durationSec;
    }
    return total;
  }
};

struct SessionState {
  bool running = false;
  bool timerActive = false;
  int32_t timerRemainingSec = 0;
};

class SessionController {
public:
  void begin(MotorController* motor);
  void tick();

  // Process control
  void start();
  void stop();
  void toggleRun();  // start/pause
  void nextStep();   // advance to next step (after pause)

  // Settings access
  SessionSettings& settings() { return _settings; }
  const SessionSettings& settings() const { return _settings; }

  // State queries
  bool isRunning() const { return _running; }
  bool isPaused() const { return _paused; }
  bool isTimerActive() const { return _timerActive; }
  int8_t currentStep() const { return _currentStep; }
  int32_t stepRemainingSec() const;
  int32_t totalRemainingSec() const;

  // Apply settings to motor
  void applyToMotor();

  // Temperature for coefficient calculation
  void setCurrentTemp(float tempC) { _currentTempC = tempC; }
  float currentTemp() const { return _currentTempC; }
  
  // Get adjusted values (with temp coefficient applied)
  float adjustedRpm() const;
  int32_t adjustedStepDurationSec(int8_t stepIdx) const;
  
  // Temperature alarm
  bool isTempAlarm() const { return _tempAlarm; }
  bool isTempLow() const { return _tempLow; }
  bool isTempHigh() const { return _tempHigh; }

private:
  MotorController* _motor = nullptr;
  SessionSettings _settings;

  // Runtime state
  bool _running = false;
  bool _paused = false;       // paused between steps
  bool _timerActive = false;
  int8_t _currentStep = 0;    // current step index (0-based)
  uint32_t _stepStartMs = 0;
  uint32_t _stepPausedMs = 0;
  float _currentTempC = NAN;
  bool _tempAlarm = false;
  bool _tempLow = false;
  bool _tempHigh = false;

  void checkTempLimits();
  float calcTempCoefMultiplier() const;
  void updateTimer();
  void resetTimer();
};
