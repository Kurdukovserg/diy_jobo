#include "App.h"
#include "Config.h"
#include "StepperISR.h"
#include <Wire.h>

App* App::instance = nullptr;

void App::buzzerTestCallback() {
  if (instance) instance->_buzzer.testBeep();
}

void App::begin() {
  instance = this;
#if !defined(ESP32)
  Wire.begin(PIN_SDA, PIN_SCL);
#else
  Wire.begin(PIN_SDA, PIN_SCL);
#endif
  Wire.setClock(400000);

  _in.begin();
  _ui.begin();
  _temp.begin(PIN_DS18B20);

  // Motor init
  stepperISR.begin(PIN_STEP, PIN_DIR);
  MotorConfig mcfg;
  mcfg.stepsPerRev = STEPS_PER_REV;
  mcfg.microsteps = MICROSTEPS;
  mcfg.accelRpmPerSec = 60.0f;
  mcfg.reverseEnabled = false;
  mcfg.reverseEverySec = 0.0f;
  _motor.begin(mcfg);
  
  _session.begin(&_motor);
  _menu.begin(&_session);
  _menu.setBuzzerTestCallback(&App::buzzerTestCallback);
  
  Buzzer::Config bcfg;
  bcfg.pin = PIN_BUZZER;
  _buzzer.begin(bcfg);
}

void App::tick() {
  InputsSnapshot s = _in.tick();
  bool settingsChanged = _menu.handleInput(s);
  _temp.tick();
  _session.setCurrentTemp(_temp.tempC());
  _session.tick();
  _buzzer.tick();
  
  // Sync settings from menu when changed
  if (settingsChanged) {
    // Buzzer event settings
    BuzzerSettings bs;
    bs.enabled = _menu.editBuzzerEnabled();
    bs.onStepFinished = _menu.editBuzzerStepFinished();
    bs.onProcessEnded = _menu.editBuzzerProcessEnded();
    bs.onTempWarning = _menu.editBuzzerTempWarning();
    bs.freqHz = _menu.editBuzzerFreq();
    _buzzer.setSettings(bs);
    
    // Hardware settings
    const auto& hw = _menu.hwSettings();
    _motor.setStepsPerRev(hw.stepsPerRev);
    _motor.setMicrosteps(hw.microsteps);
    _buzzer.setActiveHigh(hw.buzzerActiveHigh);
    _temp.setOffset(hw.tempOffset);
  }
  
  checkBuzzerEvents();
  updateUiModel(s);
  _ui.tick(_uiModel);
}

void App::updateUiModel(const InputsSnapshot& s) {
  Screen scr = _menu.screen();
  _uiModel.screen = scr;
  _uiModel.menuIdx = _menu.menuIdx();
  _uiModel.subMenuIdx = _menu.subMenuIdx();
  
  const auto& set = _session.settings();
  
  _uiModel.run = _session.isRunning();
  _uiModel.currentRpm = _motor.currentRpm();
  _uiModel.dirFwd = _motor.dirFwd();
  
  // RPM
  _uiModel.rpm = (scr == Screen::EditRpm) ? _menu.editRpm() : set.targetRpm;
  _uiModel.adjustedRpm = _session.adjustedRpm();
  
  // Reverse
  _uiModel.reverseEnabled = (scr == Screen::EditReverseEnabled) ? _menu.editReverseEnabled() : set.reverseEnabled;
  _uiModel.reverseIntervalSec = (scr == Screen::EditReverseInterval) ? _menu.editReverseInterval() : set.reverseIntervalSec;

  // Process steps
  _uiModel.stepCount = set.stepCount;
  _uiModel.currentStep = _session.currentStep();
  _uiModel.stepRemainingSec = _session.stepRemainingSec();
  _uiModel.totalRemainingSec = _session.totalRemainingSec();
  _uiModel.isPaused = _session.isPaused();
  for (int8_t i = 0; i < set.stepCount && i < 10; i++) {
    _uiModel.stepDurations[i] = set.steps[i].durationSec;
  }
  
  // Edit step values
  _uiModel.editStepIdx = _menu.editStepIdx();
  _uiModel.editStepDuration = (scr == Screen::EditStepDuration) ? _menu.editStepDuration() : 0;

  // Temperature coefficient
  _uiModel.tempCoefEnabled = (scr == Screen::EditTempCoefEnabled) ? _menu.editTempCoefEnabled() : set.tempCoefEnabled;
  _uiModel.tempCoefBase = (scr == Screen::EditTempCoefBase) ? _menu.editTempCoefBase() : set.tempCoefBase;
  _uiModel.tempCoefPercent = (scr == Screen::EditTempCoefPercent) ? _menu.editTempCoefPercent() : set.tempCoefPercent;
  _uiModel.tempCoefTarget = (scr == Screen::EditTempCoefTarget) ? _menu.editTempCoefTarget() : set.tempCoefTarget;

  // Temperature limits
  _uiModel.tempLimitsEnabled = (scr == Screen::EditTempLimitsEnabled) ? _menu.editTempLimitsEnabled() : set.tempLimitsEnabled;
  _uiModel.tempMin = (scr == Screen::EditTempMin) ? _menu.editTempMin() : set.tempMin;
  _uiModel.tempMax = (scr == Screen::EditTempMax) ? _menu.editTempMax() : set.tempMax;
  _uiModel.tempAlarm = _session.isTempAlarm();
  _uiModel.tempLow = _session.isTempLow();
  _uiModel.tempHigh = _session.isTempHigh();

  // Buzzer settings
  const auto& bset = _buzzer.settings();
  _uiModel.buzzerEnabled = (scr == Screen::EditBuzzerEnabled) ? _menu.editBuzzerEnabled() : bset.enabled;
  _uiModel.buzzerStepFinished = (scr == Screen::EditBuzzerStepFinished) ? _menu.editBuzzerStepFinished() : bset.onStepFinished;
  _uiModel.buzzerProcessEnded = (scr == Screen::EditBuzzerProcessEnded) ? _menu.editBuzzerProcessEnded() : bset.onProcessEnded;
  _uiModel.buzzerTempWarning = (scr == Screen::EditBuzzerTempWarning) ? _menu.editBuzzerTempWarning() : bset.onTempWarning;
  _uiModel.buzzerFreq = (scr == Screen::EditBuzzerFreq) ? _menu.editBuzzerFreq() : bset.freqHz;

  // Hardware settings
  const auto& hw = _menu.hwSettings();
  _uiModel.stepsPerRev = (scr == Screen::EditStepsPerRev) ? _menu.editStepsPerRev() : hw.stepsPerRev;
  _uiModel.microsteps = (scr == Screen::EditMicrosteps) ? _menu.editMicrosteps() : hw.microsteps;
  _uiModel.driverType = (scr == Screen::EditDriverType) ? (uint8_t)_menu.editDriverType() : (uint8_t)hw.driverType;
  _uiModel.motorInvert = (scr == Screen::EditMotorInvert) ? _menu.editMotorInvert() : hw.motorInvertDir;
  _uiModel.buzzerType = (scr == Screen::EditBuzzerType) ? (uint8_t)_menu.editBuzzerType() : (uint8_t)hw.buzzerType;
  _uiModel.buzzerActiveHigh = (scr == Screen::EditBuzzerActiveHigh) ? _menu.editBuzzerActiveHigh() : hw.buzzerActiveHigh;
  _uiModel.tempOffset = (scr == Screen::EditTempOffset) ? _menu.editTempOffset() : hw.tempOffset;

  _uiModel.hasTemp = _temp.hasSensor();
  _uiModel.tempC = _temp.tempC();

  // Debug
  _uiModel.okDown = s.okDown;
  _uiModel.backDown = s.backDown;
  _uiModel.a0Down = s.a0BackDown;
  _uiModel.encSwDown = s.encSwDown;
  _uiModel.okEvent = s.okPressed;
  _uiModel.encSwEvent = s.encSwPressed;
  _uiModel.backEvent = s.backPressed;
  _uiModel.a0BackEvent = s.a0BackPressed;
  _uiModel.encSwRawHigh = s.encSwRawHigh;
}

void App::checkBuzzerEvents() {
  bool running = _session.isRunning();
  int8_t step = _session.currentStep();
  bool paused = _session.isPaused();
  bool tempAlarm = _session.isTempAlarm();
  
  // Step completed - entered pause state between steps
  if (paused && !_prevPaused && _prevRunning) {
    _buzzer.alert(AlertType::StepFinished);
  }
  
  // Session/process completed - was running, now stopped (not paused)
  if (_prevRunning && !running && !paused) {
    _buzzer.alert(AlertType::ProcessEnded);
  }
  
  // Temperature alarm started
  if (tempAlarm && !_prevTempAlarm) {
    _buzzer.alert(AlertType::TempWarning);
  }
  // Temperature alarm cleared
  if (!tempAlarm && _prevTempAlarm) {
    _buzzer.stopAlert();
  }
  
  _prevStep = step;
  _prevRunning = running;
  _prevPaused = paused;
  _prevTempAlarm = tempAlarm;
}
