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

private:
  Inputs _in;
  Ui _ui;
  TempSensor _temp;
  MotorController _motor;
  SessionController _session;
  MenuController _menu;

  UiModel _uiModel{};

  void updateUiModel(const InputsSnapshot& s);
};
