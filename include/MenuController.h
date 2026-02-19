#pragma once
#include <Arduino.h>
#include "Inputs.h"
#include "SessionController.h"

enum class Screen : uint8_t {
  Main,
  Menu,
  EditRpm,
  // Steps submenu (development profile)
  StepsMenu,
  EditStepDuration,
  // Reverse submenu
  ReverseMenu,
  EditReverseEnabled,
  EditReverseInterval,
  // TempCoef submenu
  TempCoefMenu,
  EditTempCoefEnabled,
  EditTempCoefBase,
  EditTempCoefPercent,
  EditTempCoefTarget,
  // Temp limits submenu
  EditTempLimitsEnabled,
  EditTempMin,
  EditTempMax,
  // Buzzer submenu
  BuzzerMenu,
  EditBuzzerEnabled
};

class MenuController {
public:
  void begin(SessionController* session);
  
  bool handleInput(const InputsSnapshot& s);
  
  // State queries
  Screen screen() const { return _screen; }
  int8_t menuIdx() const { return _menuIdx; }
  int8_t subMenuIdx() const { return _subMenuIdx; }
  
  // Edit values for UI display
  int32_t editRpm() const { return _editRpm; }
  int32_t editStepDuration() const { return _editStepDuration; }
  int8_t editStepIdx() const { return _editStepIdx; }
  bool editReverseEnabled() const { return _editReverseEnabled; }
  float editReverseInterval() const { return _editReverseInterval; }
  bool editTempCoefEnabled() const { return _editTempCoefEnabled; }
  float editTempCoefBase() const { return _editTempCoefBase; }
  float editTempCoefPercent() const { return _editTempCoefPercent; }
  TempCoefTarget editTempCoefTarget() const { return _editTempCoefTarget; }
  bool editTempLimitsEnabled() const { return _editTempLimitsEnabled; }
  float editTempMin() const { return _editTempMin; }
  float editTempMax() const { return _editTempMax; }

private:
  SessionController* _session = nullptr;
  
  Screen _screen = Screen::Main;
  int8_t _menuIdx = 0;
  int8_t _subMenuIdx = 0;
  static constexpr int8_t MENU_ITEMS = 5;  // RPM, Steps, Reverse, TempCoef, Buzzer
  
  // Temporary edit values
  int32_t _editRpm = 0;
  int32_t _editStepDuration = 0;
  int8_t _editStepIdx = 0;  // which step is being edited
  bool _editReverseEnabled = false;
  float _editReverseInterval = 0.0f;
  bool _editTempCoefEnabled = false;
  float _editTempCoefBase = 20.0f;
  float _editTempCoefPercent = 10.0f;
  TempCoefTarget _editTempCoefTarget = TempCoefTarget::Timer;
  bool _editTempLimitsEnabled = false;
  float _editTempMin = 18.0f;
  float _editTempMax = 24.0f;

  static constexpr int RPM_MIN = 1;
  static constexpr int RPM_MAX = 80;

  void handleMainScreen(const InputsSnapshot& s);
  void handleMenuScreen(const InputsSnapshot& s);
  bool handleEditRpm(const InputsSnapshot& s);
  // Steps
  void handleStepsMenu(const InputsSnapshot& s);
  bool handleEditStepDuration(const InputsSnapshot& s);
  // Reverse
  void handleReverseMenu(const InputsSnapshot& s);
  bool handleEditReverseEnabled(const InputsSnapshot& s);
  bool handleEditReverseInterval(const InputsSnapshot& s);
  // TempCoef
  void handleTempCoefMenu(const InputsSnapshot& s);
  bool handleEditTempCoefEnabled(const InputsSnapshot& s);
  bool handleEditTempCoefBase(const InputsSnapshot& s);
  bool handleEditTempCoefPercent(const InputsSnapshot& s);
  bool handleEditTempCoefTarget(const InputsSnapshot& s);
  // Temp limits
  bool handleEditTempLimitsEnabled(const InputsSnapshot& s);
  bool handleEditTempMin(const InputsSnapshot& s);
  bool handleEditTempMax(const InputsSnapshot& s);
  // Buzzer
  void handleBuzzerMenu(const InputsSnapshot& s);
  bool handleEditBuzzerEnabled(const InputsSnapshot& s);
};
