#include "App.h"
#include "Config.h"
#include "StepperISR.h"
#include <Wire.h>

void App::begin() {
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
}

void App::tick() {
  InputsSnapshot s = _in.tick();
  _menu.handleInput(s);
  _temp.tick();
  _session.tick();
  
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
  
  // Reverse
  _uiModel.reverseEnabled = (scr == Screen::EditReverseEnabled) ? _menu.editReverseEnabled() : set.reverseEnabled;
  _uiModel.reverseIntervalSec = (scr == Screen::EditReverseInterval) ? _menu.editReverseInterval() : set.reverseIntervalSec;

  // Process steps
  _uiModel.stepCount = set.stepCount;
  _uiModel.currentStep = _session.currentStep();
  _uiModel.stepRemainingSec = _session.stepRemainingSec();
  _uiModel.totalRemainingSec = _session.totalRemainingSec();
  _uiModel.isPaused = _session.isPaused();
  
  // Edit step values
  _uiModel.editStepIdx = _menu.editStepIdx();
  _uiModel.editStepDuration = (scr == Screen::EditStepDuration) ? _menu.editStepDuration() : 0;

  // Temperature coefficient
  _uiModel.tempCoefEnabled = (scr == Screen::EditTempCoefEnabled) ? _menu.editTempCoefEnabled() : set.tempCoefEnabled;
  _uiModel.tempCoefBase = (scr == Screen::EditTempCoefBase) ? _menu.editTempCoefBase() : set.tempCoefBase;
  _uiModel.tempCoefPercent = (scr == Screen::EditTempCoefPercent) ? _menu.editTempCoefPercent() : set.tempCoefPercent;
  _uiModel.tempCoefTarget = (scr == Screen::EditTempCoefTarget) ? _menu.editTempCoefTarget() : set.tempCoefTarget;

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
