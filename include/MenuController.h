#pragma once
#include <Arduino.h>
#include "Inputs.h"
#include "SessionController.h"
#include "HardwareSettings.h"

enum class Screen : uint8_t {
  Main,
  Menu,
  // Profile submenu (development profile)
  ProfileMenu,         // Profile selection (Custom for now, future: Default, Profile1, etc.)
  ProfileEditMenu,     // Settings/Steps for selected profile
  ProfileSettingsMenu, // Profile-level settings (TempCoef, TempLimits, etc.)
  StepsMenu,           // List of steps
  StepDetailMenu,      // sub-menu for single step
  EditStepDuration,
  EditStepRpm,
  EditStepTempMode,
  EditStepTempTarget,
  EditStepTempBiasMode,
  EditStepTempBias,
  EditStepName,
  // Step-level temp coef override
  EditStepTempCoefOverride,
  EditStepTempCoefEnabled,
  EditStepTempCoefBase,
  EditStepTempCoefPercent,
  EditStepTempCoefTarget,
  EditStepTempAlarmAction,
  // Profile-level TempCoef (inside ProfileSettingsMenu)
  EditTempCoefEnabled,
  EditTempCoefBase,
  EditTempCoefPercent,
  EditTempCoefTarget,
  EditTempAlarmAction,
  // Temp limits (inside ProfileSettingsMenu)
  EditTempLimitsEnabled,
  EditTempMin,
  EditTempMax,
  // Reverse submenu
  ReverseMenu,
  EditReverseEnabled,
  EditReverseInterval,
  // Buzzer submenu
  BuzzerMenu,
  EditBuzzerEnabled,
  EditBuzzerStepFinished,
  EditBuzzerProcessEnded,
  EditBuzzerTempWarning,
  EditBuzzerFreq,
  BuzzerTest,
  // Hardware submenu
  HardwareMenu,
  EditStepsPerRev,
  EditMicrosteps,
  EditDriverType,
  EditMotorInvert,
  EditBuzzerType,
  EditBuzzerActiveHigh,
  EditTempOffset
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
  int32_t editStepDuration() const { return _editStepDuration; }
  int8_t editStepIdx() const { return _editStepIdx; }
  int32_t editStepRpm() const { return _editStepRpm; }
  StepTempMode editStepTempMode() const { return _editStepTempMode; }
  float editStepTempTarget() const { return _editStepTempTarget; }
  StepTempBiasMode editStepTempBiasMode() const { return _editStepTempBiasMode; }
  float editStepTempBias() const { return _editStepTempBias; }
  const char* editStepName() const { return _editStepName; }
  int8_t editStepDetailIdx() const { return _editStepDetailIdx; }
  bool editReverseEnabled() const { return _editReverseEnabled; }
  float editReverseInterval() const { return _editReverseInterval; }
  bool editTempCoefEnabled() const { return _editTempCoefEnabled; }
  float editTempCoefBase() const { return _editTempCoefBase; }
  float editTempCoefPercent() const { return _editTempCoefPercent; }
  TempCoefTarget editTempCoefTarget() const { return _editTempCoefTarget; }
  TempAlarmAction editTempAlarmAction() const { return _editTempAlarmAction; }
  bool editStepTempCoefOverride() const { return _editStepTempCoefOverride; }
  bool editTempLimitsEnabled() const { return _editTempLimitsEnabled; }
  float editTempMin() const { return _editTempMin; }
  float editTempMax() const { return _editTempMax; }
  // Buzzer
  bool editBuzzerEnabled() const { return _editBuzzerEnabled; }
  bool editBuzzerStepFinished() const { return _editBuzzerStepFinished; }
  bool editBuzzerProcessEnded() const { return _editBuzzerProcessEnded; }
  bool editBuzzerTempWarning() const { return _editBuzzerTempWarning; }
  uint16_t editBuzzerFreq() const { return _editBuzzerFreq; }
  // Hardware
  uint16_t editStepsPerRev() const { return _editStepsPerRev; }
  uint8_t editMicrosteps() const { return _editMicrosteps; }
  DriverType editDriverType() const { return _editDriverType; }
  bool editMotorInvert() const { return _editMotorInvert; }
  BuzzerType editBuzzerType() const { return _editBuzzerType; }
  bool editBuzzerActiveHigh() const { return _editBuzzerActiveHigh; }
  float editTempOffset() const { return _editTempOffset; }
  
  // Hardware settings storage (owned by MenuController for now)
  HardwareSettings& hwSettings() { return _hwSettings; }
  const HardwareSettings& hwSettings() const { return _hwSettings; }
  
  // Callback for buzzer test (set by App)
  void setBuzzerTestCallback(void (*cb)()) { _buzzerTestCb = cb; }
  void setHardwareChangedCallback(void (*cb)()) { _hwChangedCb = cb; }

private:
  SessionController* _session = nullptr;
  
  Screen _screen = Screen::Main;
  int8_t _menuIdx = 0;
  int8_t _subMenuIdx = 0;
  static constexpr int8_t MENU_ITEMS = 4;  // Profile, Reverse, Buzzer, Hardware
  
  // Temporary edit values
  int32_t _editStepDuration = 0;
  int8_t _editStepIdx = 0;  // which step is being edited
  int8_t _editStepDetailIdx = 0;  // which item in step detail menu
  int32_t _editStepRpm = 30;
  StepTempMode _editStepTempMode = StepTempMode::Off;
  float _editStepTempTarget = 20.0f;
  StepTempBiasMode _editStepTempBiasMode = StepTempBiasMode::Off;
  float _editStepTempBias = 2.0f;
  char _editStepName[STEP_NAME_LEN] = "";
  int8_t _editNameCursor = 0;  // cursor position for name editing
  bool _editReverseEnabled = false;
  float _editReverseInterval = 0.0f;
  bool _editTempCoefEnabled = false;
  float _editTempCoefBase = 20.0f;
  float _editTempCoefPercent = 10.0f;
  TempCoefTarget _editTempCoefTarget = TempCoefTarget::Timer;
  TempAlarmAction _editTempAlarmAction = TempAlarmAction::Beep;
  bool _editStepTempCoefOverride = false;
  bool _editTempLimitsEnabled = false;
  float _editTempMin = 18.0f;
  float _editTempMax = 24.0f;
  // Buzzer
  bool _editBuzzerEnabled = true;
  bool _editBuzzerStepFinished = true;
  bool _editBuzzerProcessEnded = true;
  bool _editBuzzerTempWarning = true;
  uint16_t _editBuzzerFreq = 2200;
  // Hardware
  HardwareSettings _hwSettings;
  uint16_t _editStepsPerRev = 200;
  uint8_t _editMicrosteps = 16;
  DriverType _editDriverType = DriverType::TMC2209;
  bool _editMotorInvert = false;
  BuzzerType _editBuzzerType = BuzzerType::Passive;
  bool _editBuzzerActiveHigh = true;
  float _editTempOffset = 0.0f;

  static constexpr int RPM_MIN = 1;
  static constexpr int RPM_MAX = 80;

  void handleMainScreen(const InputsSnapshot& s);
  void handleMenuScreen(const InputsSnapshot& s);
  // Steps
  void handleProfileMenu(const InputsSnapshot& s);
  void handleProfileEditMenu(const InputsSnapshot& s);
  void handleProfileSettingsMenu(const InputsSnapshot& s);
  void handleStepsMenu(const InputsSnapshot& s);
  void handleStepDetailMenu(const InputsSnapshot& s);
  bool handleEditStepDuration(const InputsSnapshot& s);
  bool handleEditStepRpm(const InputsSnapshot& s);
  bool handleEditStepTempMode(const InputsSnapshot& s);
  bool handleEditStepTempTarget(const InputsSnapshot& s);
  bool handleEditStepTempBiasMode(const InputsSnapshot& s);
  bool handleEditStepTempBias(const InputsSnapshot& s);
  bool handleEditStepName(const InputsSnapshot& s);
  // Step-level temp coef override
  bool handleEditStepTempCoefOverride(const InputsSnapshot& s);
  bool handleEditStepTempCoefEnabled(const InputsSnapshot& s);
  bool handleEditStepTempCoefBase(const InputsSnapshot& s);
  bool handleEditStepTempCoefPercent(const InputsSnapshot& s);
  bool handleEditStepTempCoefTarget(const InputsSnapshot& s);
  bool handleEditStepTempAlarmAction(const InputsSnapshot& s);
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
  bool handleEditTempAlarmAction(const InputsSnapshot& s);
  // Temp limits
  bool handleEditTempLimitsEnabled(const InputsSnapshot& s);
  bool handleEditTempMin(const InputsSnapshot& s);
  bool handleEditTempMax(const InputsSnapshot& s);
  // Buzzer
  void handleBuzzerMenu(const InputsSnapshot& s);
  bool handleEditBuzzerEnabled(const InputsSnapshot& s);
  bool handleEditBuzzerStepFinished(const InputsSnapshot& s);
  bool handleEditBuzzerProcessEnded(const InputsSnapshot& s);
  bool handleEditBuzzerTempWarning(const InputsSnapshot& s);
  bool handleEditBuzzerFreq(const InputsSnapshot& s);
  void handleBuzzerTest(const InputsSnapshot& s);
  // Hardware
  void handleHardwareMenu(const InputsSnapshot& s);
  bool handleEditStepsPerRev(const InputsSnapshot& s);
  bool handleEditMicrosteps(const InputsSnapshot& s);
  bool handleEditDriverType(const InputsSnapshot& s);
  bool handleEditMotorInvert(const InputsSnapshot& s);
  bool handleEditBuzzerType(const InputsSnapshot& s);
  bool handleEditBuzzerActiveHigh(const InputsSnapshot& s);
  bool handleEditTempOffset(const InputsSnapshot& s);
  
  void (*_buzzerTestCb)() = nullptr;
  void (*_hwChangedCb)() = nullptr;
};
