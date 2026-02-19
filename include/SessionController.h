#pragma once
#include <Arduino.h>
#include "MotorController.h"

enum class TempCoefTarget : uint8_t {
  Timer = 0,
  Rpm = 1,
  Both = 2
};

// Action when temperature limit is hit
enum class TempAlarmAction : uint8_t {
  None = 0,     // no action, just display
  Beep = 1,     // beep warning
  Pause = 2,    // pause process
  Stop = 3      // stop process completely
};

static constexpr int8_t MAX_STEPS = 10;

static constexpr int8_t STEP_NAME_LEN = 12;  // max chars for step name
static constexpr int8_t PROFILE_NAME_LEN = 16;  // max chars for profile name

// Temperature mode for per-step temp control
enum class StepTempMode : uint8_t {
  Off = 0,      // no temp control for this step
  Target = 1    // target temperature (display only, no control yet)
};

// Temperature bias/limit mode for per-step
enum class StepTempBiasMode : uint8_t {
  Off = 0,      // no bias/limit
  Bias = 1      // +/- bias from target
};

struct ProcessStep {
  int32_t durationSec = 0;        // 0 = disabled/unused
  int32_t rpm = 30;               // RPM for this step
  StepTempMode tempMode = StepTempMode::Off;
  float tempTarget = 20.0f;       // target temp if tempMode == Target
  StepTempBiasMode tempBiasMode = StepTempBiasMode::Off;
  float tempBias = 2.0f;          // +/- bias tolerance
  char name[STEP_NAME_LEN] = ""; // step name (empty = "Step N")
  
  // Per-step temp coefficient override (UseProfile = use profile settings)
  bool tempCoefOverride = false;  // if true, use step-specific settings below
  bool tempCoefEnabled = false;
  float tempCoefBase = 20.0f;
  float tempCoefPercent = 10.0f;
  TempCoefTarget tempCoefTarget = TempCoefTarget::Timer;
  TempAlarmAction tempAlarmAction = TempAlarmAction::Beep;
};

struct SessionSettings {
  int32_t targetRpm = 30;  // global RPM (used when no step-specific RPM)
  char profileName[PROFILE_NAME_LEN] = "";  // profile name (empty = no profile)
  
  // Reverse
  bool reverseEnabled = true;
  float reverseIntervalSec = 10.0f;
  
  // Process steps (development profile)
  ProcessStep steps[MAX_STEPS];
  int8_t stepCount = 1;  // number of active steps (1-10)
  
  // Profile-level temperature coefficient (defaults for all steps)
  bool tempCoefEnabled = false;
  float tempCoefBase = 20.0f;
  float tempCoefPercent = 10.0f;
  TempCoefTarget tempCoefTarget = TempCoefTarget::Timer;
  TempAlarmAction tempAlarmAction = TempAlarmAction::Beep;
  
  // Temperature limits (alarm if outside range) - profile level
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
  float calcTempCoefMultiplierForStep(int8_t stepIdx) const;
  void updateTimer();
  void resetTimer();
};
