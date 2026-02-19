#pragma once
#include <cstdint>
#include <Arduino.h>
#include "Inputs.h"
#include "Ui.h"
#include "TempSensor.h"
#include "MotorController.h"
#include "SessionController.h"
#include "MenuController.h"
#include "Buzzer.h"

class App {
public:
  void begin();
  void tick();
  
  static App* instance;
  static void buzzerTestCallback();

private:
  Inputs _in;
  Ui _ui;
  TempSensor _temp;
  MotorController _motor;
  SessionController _session;
  MenuController _menu;
  Buzzer _buzzer;

  UiModel _uiModel{};
  
  bool _prevTempAlarm = false;
  int8_t _prevStep = -1;
  bool _prevRunning = false;
  bool _prevPaused = false;

  void updateUiModel(const InputsSnapshot& s);
  void checkBuzzerEvents();
};
