// Self-contained test - mocks and SessionController logic inline
#include "Arduino.h"
#include <unity.h>

// === Mock MotorController ===
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
  void setRun(bool run) { _run = run; }
  void setTargetRpm(float rpm) { _targetRpm = rpm; }
  void setReverseEnabled(bool en) {}
  void setReverseEverySec(float sec) {}
  void tick() {}
  
  bool _run = false;
  float _targetRpm = 0;
  void reset() { _run = false; _targetRpm = 0; }
};

// === SessionController (simplified copy for testing) ===
struct StepConfig { int32_t durationSec = 0; };

struct SessionSettings {
  int16_t targetRpm = 30;
  int8_t stepCount = 1;
  StepConfig steps[8];
  bool reverseEnabled = true;
  float reverseIntervalSec = 10.0f;
  bool tempCoefEnabled = false;
  float tempCoefBase = 20.0f;
  float tempCoefPercent = 0.0f;
};

class SessionController {
public:
  void begin(MotorController* motor) {
    _motor = motor;
    _running = false;
    _paused = false;
    _currentStep = 0;
  }
  
  void tick() {
    if (!_motor) return;
    updateTimer();
    applyToMotor();
  }
  
  void toggleRun() {
    if (_running) {
      if (_timerActive) {
        _stepPausedMs += millis() - _stepStartMs;
        _timerActive = false;
      }
      _running = false;
    } else if (_paused) {
      nextStep();
    } else {
      _running = true;
      int32_t stepDur = _settings.steps[_currentStep].durationSec;
      if (stepDur > 0 && !_timerActive) {
        _timerActive = true;
        _stepStartMs = millis();
      }
    }
  }
  
  void stop() {
    _running = false;
    _paused = false;
    _timerActive = false;
    _currentStep = 0;
    _stepPausedMs = 0;
  }
  
  bool isRunning() const { return _running; }
  bool isPaused() const { return _paused; }
  int8_t currentStep() const { return _currentStep; }
  SessionSettings& settings() { return _settings; }
  
private:
  void nextStep() {
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
      stop();
    }
  }
  
  void updateTimer() {
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
      _timerActive = false;
      _stepPausedMs = 0;
      
      if (_currentStep + 1 < _settings.stepCount) {
        _running = false;
        _paused = true;
      } else {
        stop();
      }
    }
  }
  
  void applyToMotor() {
    if (!_motor) return;
    _motor->setRun(_running && !_paused);
    _motor->setTargetRpm((float)_settings.targetRpm);
  }
  
  MotorController* _motor = nullptr;
  SessionSettings _settings{};
  bool _running = false;
  bool _paused = false;
  int8_t _currentStep = 0;
  bool _timerActive = false;
  uint32_t _stepStartMs = 0;
  uint32_t _stepPausedMs = 0;
};

MotorController mockMotor;
SessionController session;

void setUp(void) {
  setMockMillis(0);
  mockMotor.reset();
  session.begin(&mockMotor);
}

void tearDown(void) {
}

// Test: Initial state
void test_initial_state(void) {
  TEST_ASSERT_FALSE(session.isRunning());
  TEST_ASSERT_FALSE(session.isPaused());
  TEST_ASSERT_EQUAL(0, session.currentStep());
}

// Test: Start session
void test_start_session(void) {
  auto& settings = session.settings();
  settings.targetRpm = 30;
  settings.stepCount = 2;
  settings.steps[0].durationSec = 10;
  settings.steps[1].durationSec = 20;
  
  session.toggleRun();
  
  TEST_ASSERT_TRUE(session.isRunning());
  TEST_ASSERT_FALSE(session.isPaused());
  TEST_ASSERT_EQUAL(0, session.currentStep());
}

// Test: Pause mid-step
void test_pause_mid_step(void) {
  auto& settings = session.settings();
  settings.targetRpm = 30;
  settings.stepCount = 1;
  settings.steps[0].durationSec = 60;
  
  session.toggleRun();  // Start
  TEST_ASSERT_TRUE(session.isRunning());
  
  advanceMockMillis(5000);  // 5 seconds
  session.tick();
  
  session.toggleRun();  // Pause
  TEST_ASSERT_FALSE(session.isRunning());
  TEST_ASSERT_FALSE(session.isPaused());  // Mid-step pause, not end-of-step pause
}

// Test: Step completion triggers pause
void test_step_completion_triggers_pause(void) {
  auto& settings = session.settings();
  settings.targetRpm = 30;
  settings.stepCount = 2;
  settings.steps[0].durationSec = 10;
  settings.steps[1].durationSec = 20;
  
  session.toggleRun();  // Start step 0
  TEST_ASSERT_TRUE(session.isRunning());
  TEST_ASSERT_EQUAL(0, session.currentStep());
  
  // Advance past step 0 duration
  advanceMockMillis(11000);  // 11 seconds
  session.tick();
  
  // Should be paused waiting for user to continue
  TEST_ASSERT_FALSE(session.isRunning());
  TEST_ASSERT_TRUE(session.isPaused());
  TEST_ASSERT_EQUAL(0, session.currentStep());  // Still on step 0
}

// Test: Resume after step pause goes to next step
void test_resume_after_pause_goes_to_next_step(void) {
  auto& settings = session.settings();
  settings.targetRpm = 30;
  settings.stepCount = 2;
  settings.steps[0].durationSec = 10;
  settings.steps[1].durationSec = 20;
  
  session.toggleRun();  // Start step 0
  
  advanceMockMillis(11000);
  session.tick();  // Step 0 completes, enters pause
  
  TEST_ASSERT_TRUE(session.isPaused());
  
  session.toggleRun();  // Resume -> should go to step 1
  
  TEST_ASSERT_TRUE(session.isRunning());
  TEST_ASSERT_FALSE(session.isPaused());
  TEST_ASSERT_EQUAL(1, session.currentStep());
}

// Test: Motor receives correct run state
void test_motor_receives_run_state(void) {
  auto& settings = session.settings();
  settings.targetRpm = 30;
  settings.stepCount = 1;
  settings.steps[0].durationSec = 10;
  
  session.toggleRun();  // Start
  session.tick();
  
  TEST_ASSERT_TRUE(mockMotor._run);
  TEST_ASSERT_EQUAL_FLOAT(30.0f, mockMotor._targetRpm);
}

// Test: Stop resets everything
void test_stop_resets_state(void) {
  auto& settings = session.settings();
  settings.targetRpm = 30;
  settings.stepCount = 2;
  settings.steps[0].durationSec = 10;
  settings.steps[1].durationSec = 20;
  
  session.toggleRun();  // Start
  advanceMockMillis(5000);
  session.tick();
  
  session.stop();
  
  TEST_ASSERT_FALSE(session.isRunning());
  TEST_ASSERT_FALSE(session.isPaused());
  TEST_ASSERT_EQUAL(0, session.currentStep());
}

// Test: Single step session ends properly
void test_single_step_session_ends(void) {
  auto& settings = session.settings();
  settings.targetRpm = 30;
  settings.stepCount = 1;
  settings.steps[0].durationSec = 10;
  
  session.toggleRun();  // Start
  
  advanceMockMillis(11000);
  session.tick();  // Step completes
  
  // Single step = session ends (not pause)
  TEST_ASSERT_FALSE(session.isRunning());
  TEST_ASSERT_FALSE(session.isPaused());
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  
  RUN_TEST(test_initial_state);
  RUN_TEST(test_start_session);
  RUN_TEST(test_pause_mid_step);
  RUN_TEST(test_step_completion_triggers_pause);
  RUN_TEST(test_resume_after_pause_goes_to_next_step);
  RUN_TEST(test_motor_receives_run_state);
  RUN_TEST(test_stop_resets_state);
  RUN_TEST(test_single_step_session_ends);
  
  return UNITY_END();
}
