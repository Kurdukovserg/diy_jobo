#include "MenuController.h"

void MenuController::begin(SessionController* session) {
  _session = session;
}

bool MenuController::handleInput(const InputsSnapshot& s) {
  bool settingsChanged = false;
  
  switch (_screen) {
    case Screen::Main:
      handleMainScreen(s);
      break;
    case Screen::Menu:
      handleMenuScreen(s);
      break;
    // Profile submenu
    case Screen::ProfileMenu:
      handleProfileMenu(s);
      break;
    case Screen::ProfileEditMenu:
      handleProfileEditMenu(s);
      break;
    case Screen::ProfileSettingsMenu:
      handleProfileSettingsMenu(s);
      break;
    case Screen::StepsMenu:
      handleStepsMenu(s);
      break;
    case Screen::StepDetailMenu:
      handleStepDetailMenu(s);
      break;
    case Screen::EditStepDuration:
      settingsChanged = handleEditStepDuration(s);
      break;
    case Screen::EditStepRpm:
      settingsChanged = handleEditStepRpm(s);
      break;
    case Screen::EditStepTempMode:
      settingsChanged = handleEditStepTempMode(s);
      break;
    case Screen::EditStepTempTarget:
      settingsChanged = handleEditStepTempTarget(s);
      break;
    case Screen::EditStepTempBiasMode:
      settingsChanged = handleEditStepTempBiasMode(s);
      break;
    case Screen::EditStepTempBias:
      settingsChanged = handleEditStepTempBias(s);
      break;
    case Screen::EditStepName:
      settingsChanged = handleEditStepName(s);
      break;
    // Step-level temp coef override
    case Screen::EditStepTempCoefOverride:
      settingsChanged = handleEditStepTempCoefOverride(s);
      break;
    case Screen::EditStepTempCoefEnabled:
      settingsChanged = handleEditStepTempCoefEnabled(s);
      break;
    case Screen::EditStepTempCoefBase:
      settingsChanged = handleEditStepTempCoefBase(s);
      break;
    case Screen::EditStepTempCoefPercent:
      settingsChanged = handleEditStepTempCoefPercent(s);
      break;
    case Screen::EditStepTempCoefTarget:
      settingsChanged = handleEditStepTempCoefTarget(s);
      break;
    case Screen::EditStepTempAlarmAction:
      settingsChanged = handleEditStepTempAlarmAction(s);
      break;
    // Reverse submenu
    case Screen::ReverseMenu:
      handleReverseMenu(s);
      break;
    case Screen::EditReverseEnabled:
      settingsChanged = handleEditReverseEnabled(s);
      break;
    case Screen::EditReverseInterval:
      settingsChanged = handleEditReverseInterval(s);
      break;
    // TempCoef (accessed from ProfileSettingsMenu now)
    case Screen::EditTempCoefEnabled:
      settingsChanged = handleEditTempCoefEnabled(s);
      break;
    case Screen::EditTempCoefBase:
      settingsChanged = handleEditTempCoefBase(s);
      break;
    case Screen::EditTempCoefPercent:
      settingsChanged = handleEditTempCoefPercent(s);
      break;
    case Screen::EditTempCoefTarget:
      settingsChanged = handleEditTempCoefTarget(s);
      break;
    case Screen::EditTempAlarmAction:
      settingsChanged = handleEditTempAlarmAction(s);
      break;
    // Temp limits
    case Screen::EditTempLimitsEnabled:
      settingsChanged = handleEditTempLimitsEnabled(s);
      break;
    case Screen::EditTempMin:
      settingsChanged = handleEditTempMin(s);
      break;
    case Screen::EditTempMax:
      settingsChanged = handleEditTempMax(s);
      break;
    // Buzzer
    case Screen::BuzzerMenu:
      handleBuzzerMenu(s);
      break;
    case Screen::EditBuzzerEnabled:
      settingsChanged = handleEditBuzzerEnabled(s);
      break;
    case Screen::EditBuzzerStepFinished:
      settingsChanged = handleEditBuzzerStepFinished(s);
      break;
    case Screen::EditBuzzerProcessEnded:
      settingsChanged = handleEditBuzzerProcessEnded(s);
      break;
    case Screen::EditBuzzerTempWarning:
      settingsChanged = handleEditBuzzerTempWarning(s);
      break;
    case Screen::EditBuzzerFreq:
      settingsChanged = handleEditBuzzerFreq(s);
      break;
    case Screen::BuzzerTest:
      handleBuzzerTest(s);
      break;
    // Hardware
    case Screen::HardwareMenu:
      handleHardwareMenu(s);
      break;
    case Screen::EditStepsPerRev:
      settingsChanged = handleEditStepsPerRev(s);
      break;
    case Screen::EditMicrosteps:
      settingsChanged = handleEditMicrosteps(s);
      break;
    case Screen::EditDriverType:
      settingsChanged = handleEditDriverType(s);
      break;
    case Screen::EditMotorInvert:
      settingsChanged = handleEditMotorInvert(s);
      break;
    case Screen::EditBuzzerType:
      settingsChanged = handleEditBuzzerType(s);
      break;
    case Screen::EditBuzzerActiveHigh:
      settingsChanged = handleEditBuzzerActiveHigh(s);
      break;
    case Screen::EditTempOffset:
      settingsChanged = handleEditTempOffset(s);
      break;
  }
  
  return settingsChanged;
}

void MenuController::handleMainScreen(const InputsSnapshot& s) {
  if (!_session) return;
  
  if (s.encDelta != 0) {
    auto& set = _session->settings();
    set.targetRpm += s.encDelta;
    if (set.targetRpm < RPM_MIN) set.targetRpm = RPM_MIN;
    if (set.targetRpm > RPM_MAX) set.targetRpm = RPM_MAX;
  }

  if (s.okPressed) {
    Serial.printf("OK pressed, screen=Main, paused=%d\n", _session->isPaused());
    _session->toggleRun();
  }

  if (s.encSwPressed) {
    _screen = Screen::Menu;
    _menuIdx = 0;
  }

  if (s.backPressed || s.a0BackPressed) {
    _session->stop();
  }
}

void MenuController::handleMenuScreen(const InputsSnapshot& s) {
  if (!_session) return;
  
  if (s.encDelta != 0) {
    _menuIdx += s.encDelta;
    if (_menuIdx < 0) _menuIdx = MENU_ITEMS - 1;
    if (_menuIdx >= MENU_ITEMS) _menuIdx = 0;
  }

  if (s.okPressed || s.encSwPressed) {
    _subMenuIdx = 0;
    switch (_menuIdx) {
      case 0: 
        _screen = Screen::ProfileMenu; 
        break;
      case 1: 
        _screen = Screen::ReverseMenu; 
        break;
      case 2:
        _screen = Screen::BuzzerMenu;
        break;
      case 3:
        _screen = Screen::HardwareMenu;
        break;
    }
  }

  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::Main;
  }
}

// ===== Profile Submenu =====
void MenuController::handleProfileMenu(const InputsSnapshot& s) {
  // Items: Custom (future: Default, Profile1, Profile2, etc.)
  const int8_t ITEMS = 1;
  
  if (s.encDelta != 0) {
    _subMenuIdx += s.encDelta;
    if (_subMenuIdx < 0) _subMenuIdx = ITEMS - 1;
    if (_subMenuIdx >= ITEMS) _subMenuIdx = 0;
  }

  if (s.okPressed || s.encSwPressed) {
    switch (_subMenuIdx) {
      case 0:  // Custom profile
        _subMenuIdx = 0;
        _screen = Screen::ProfileEditMenu;
        break;
      // Future: case 1: Default profile, case 2: Profile1, etc.
    }
  }

  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::Menu;
  }
}

void MenuController::handleProfileEditMenu(const InputsSnapshot& s) {
  // Items: Settings, Steps
  const int8_t ITEMS = 2;
  
  if (s.encDelta != 0) {
    _subMenuIdx += s.encDelta;
    if (_subMenuIdx < 0) _subMenuIdx = ITEMS - 1;
    if (_subMenuIdx >= ITEMS) _subMenuIdx = 0;
  }

  if (s.okPressed || s.encSwPressed) {
    switch (_subMenuIdx) {
      case 0:  // Settings
        _subMenuIdx = 0;
        _screen = Screen::ProfileSettingsMenu;
        break;
      case 1:  // Steps
        _subMenuIdx = 0;
        _screen = Screen::StepsMenu;
        break;
    }
  }

  if (s.backPressed || s.a0BackPressed) {
    _subMenuIdx = 0;
    _screen = Screen::ProfileMenu;
  }
}

void MenuController::handleProfileSettingsMenu(const InputsSnapshot& s) {
  // Items: TempCoef Enabled, Base, %, Target, Alarm Action, TempLimits Enabled, Min, Max
  const int8_t ITEMS = 8;
  
  if (s.encDelta != 0) {
    _subMenuIdx += s.encDelta;
    if (_subMenuIdx < 0) _subMenuIdx = ITEMS - 1;
    if (_subMenuIdx >= ITEMS) _subMenuIdx = 0;
  }

  if (s.okPressed || s.encSwPressed) {
    const auto& set = _session->settings();
    switch (_subMenuIdx) {
      case 0:
        _editTempCoefEnabled = set.tempCoefEnabled;
        _screen = Screen::EditTempCoefEnabled;
        break;
      case 1:
        _editTempCoefBase = set.tempCoefBase;
        _screen = Screen::EditTempCoefBase;
        break;
      case 2:
        _editTempCoefPercent = set.tempCoefPercent;
        _screen = Screen::EditTempCoefPercent;
        break;
      case 3:
        _editTempCoefTarget = set.tempCoefTarget;
        _screen = Screen::EditTempCoefTarget;
        break;
      case 4:
        _editTempAlarmAction = set.tempAlarmAction;
        _screen = Screen::EditTempAlarmAction;
        break;
      case 5:
        _editTempLimitsEnabled = set.tempLimitsEnabled;
        _screen = Screen::EditTempLimitsEnabled;
        break;
      case 6:
        _editTempMin = set.tempMin;
        _screen = Screen::EditTempMin;
        break;
      case 7:
        _editTempMax = set.tempMax;
        _screen = Screen::EditTempMax;
        break;
    }
  }

  if (s.backPressed || s.a0BackPressed) {
    _subMenuIdx = 0;
    _screen = Screen::ProfileEditMenu;
  }
}

// ===== Steps Submenu =====
void MenuController::handleStepsMenu(const InputsSnapshot& s) {
  auto& set = _session->settings();
  // Items: Step 1, Step 2, ... Step N, [+ Add Step]
  int8_t itemCount = set.stepCount + (set.stepCount < MAX_STEPS ? 1 : 0);
  
  if (s.encDelta != 0) {
    _subMenuIdx += s.encDelta;
    if (_subMenuIdx < 0) _subMenuIdx = itemCount - 1;
    if (_subMenuIdx >= itemCount) _subMenuIdx = 0;
  }

  if (s.okPressed || s.encSwPressed) {
    if (_subMenuIdx < set.stepCount) {
      // Edit existing step - go to detail menu
      _editStepIdx = _subMenuIdx;
      _editStepDetailIdx = 0;
      _screen = Screen::StepDetailMenu;
    } else if (set.stepCount < MAX_STEPS) {
      // Add new step with defaults
      auto& step = set.steps[set.stepCount];
      step.durationSec = 60;  // default 1 min
      step.rpm = 30;
      step.tempMode = StepTempMode::Off;
      step.tempTarget = 20.0f;
      step.tempBiasMode = StepTempBiasMode::Off;
      step.tempBias = 2.0f;
      step.name[0] = '\0';
      set.stepCount++;
      _editStepIdx = set.stepCount - 1;
      _editStepDetailIdx = 0;
      _screen = Screen::StepDetailMenu;
    }
  }

  // Long press on encoder: delete step (if more than 1)
  if (s.encSwLongPress && _subMenuIdx < set.stepCount && set.stepCount > 1) {
    // Remove step by shifting remaining steps
    for (int8_t i = _subMenuIdx; i < set.stepCount - 1; i++) {
      set.steps[i] = set.steps[i + 1];
    }
    set.stepCount--;
    if (_subMenuIdx >= set.stepCount) _subMenuIdx = set.stepCount - 1;
  }

  if (s.backPressed || s.a0BackPressed) {
    _subMenuIdx = 1;  // Return to Steps item in ProfileEditMenu
    _screen = Screen::ProfileEditMenu;
  }
}

// Step detail menu - shows Duration, RPM, Temp, TempBias, Name, TempCoef Override
void MenuController::handleStepDetailMenu(const InputsSnapshot& s) {
  const int8_t ITEMS = 6;  // Duration, RPM, Temp, TempBias, Name, TempCoef
  
  if (s.encDelta != 0) {
    _editStepDetailIdx += s.encDelta;
    if (_editStepDetailIdx < 0) _editStepDetailIdx = ITEMS - 1;
    if (_editStepDetailIdx >= ITEMS) _editStepDetailIdx = 0;
  }

  if (s.okPressed || s.encSwPressed) {
    const auto& step = _session->settings().steps[_editStepIdx];
    switch (_editStepDetailIdx) {
      case 0:  // Duration
        _editStepDuration = step.durationSec;
        _screen = Screen::EditStepDuration;
        break;
      case 1:  // RPM
        _editStepRpm = step.rpm;
        _screen = Screen::EditStepRpm;
        break;
      case 2:  // Temp mode
        _editStepTempMode = step.tempMode;
        _editStepTempTarget = step.tempTarget;
        _screen = Screen::EditStepTempMode;
        break;
      case 3:  // Temp bias mode
        _editStepTempBiasMode = step.tempBiasMode;
        _editStepTempBias = step.tempBias;
        _screen = Screen::EditStepTempBiasMode;
        break;
      case 4:  // Name
        strncpy(_editStepName, step.name, STEP_NAME_LEN - 1);
        _editStepName[STEP_NAME_LEN - 1] = '\0';
        _editNameCursor = strlen(_editStepName);
        _screen = Screen::EditStepName;
        break;
      case 5:  // TempCoef override
        _editStepTempCoefOverride = step.tempCoefOverride;
        _screen = Screen::EditStepTempCoefOverride;
        break;
    }
  }

  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::StepsMenu;
  }
}

bool MenuController::handleEditStepDuration(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editStepDuration += s.encDelta * 10;  // 10-second increments
    if (_editStepDuration < 0) _editStepDuration = 0;
    if (_editStepDuration > 3600) _editStepDuration = 3600;  // max 1 hour
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().steps[_editStepIdx].durationSec = _editStepDuration;
    _screen = Screen::StepDetailMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::StepDetailMenu;
  }
  
  return changed;
}

bool MenuController::handleEditStepRpm(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editStepRpm += s.encDelta;
    if (_editStepRpm < RPM_MIN) _editStepRpm = RPM_MIN;
    if (_editStepRpm > RPM_MAX) _editStepRpm = RPM_MAX;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().steps[_editStepIdx].rpm = _editStepRpm;
    _screen = Screen::StepDetailMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::StepDetailMenu;
  }
  
  return changed;
}

bool MenuController::handleEditStepTempMode(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    // Toggle between Off and Target
    _editStepTempMode = (_editStepTempMode == StepTempMode::Off) 
                        ? StepTempMode::Target : StepTempMode::Off;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().steps[_editStepIdx].tempMode = _editStepTempMode;
    if (_editStepTempMode == StepTempMode::Target) {
      // Go to temp target edit
      _screen = Screen::EditStepTempTarget;
    } else {
      _screen = Screen::StepDetailMenu;
    }
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::StepDetailMenu;
  }
  
  return changed;
}

bool MenuController::handleEditStepTempTarget(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editStepTempTarget += s.encDelta * 0.5f;
    if (_editStepTempTarget < 15.0f) _editStepTempTarget = 15.0f;
    if (_editStepTempTarget > 40.0f) _editStepTempTarget = 40.0f;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().steps[_editStepIdx].tempTarget = _editStepTempTarget;
    _screen = Screen::StepDetailMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::StepDetailMenu;
  }
  
  return changed;
}

bool MenuController::handleEditStepTempBiasMode(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    // Toggle between Off and Bias
    _editStepTempBiasMode = (_editStepTempBiasMode == StepTempBiasMode::Off) 
                            ? StepTempBiasMode::Bias : StepTempBiasMode::Off;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().steps[_editStepIdx].tempBiasMode = _editStepTempBiasMode;
    if (_editStepTempBiasMode == StepTempBiasMode::Bias) {
      // Go to bias value edit
      _screen = Screen::EditStepTempBias;
    } else {
      _screen = Screen::StepDetailMenu;
    }
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::StepDetailMenu;
  }
  
  return changed;
}

bool MenuController::handleEditStepTempBias(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editStepTempBias += s.encDelta * 0.5f;
    if (_editStepTempBias < 0.5f) _editStepTempBias = 0.5f;
    if (_editStepTempBias > 10.0f) _editStepTempBias = 10.0f;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().steps[_editStepIdx].tempBias = _editStepTempBias;
    _screen = Screen::StepDetailMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::StepDetailMenu;
  }
  
  return changed;
}

bool MenuController::handleEditStepName(const InputsSnapshot& s) {
  bool changed = false;
  
  // Simple character editing: encoder cycles through characters at cursor
  if (s.encDelta != 0) {
    // Character set: A-Z, a-z, 0-9, space, and some symbols
    static const char charset[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.";
    static const int charsetLen = sizeof(charset) - 1;
    
    char current = _editStepName[_editNameCursor];
    int idx = 0;
    for (int i = 0; i < charsetLen; i++) {
      if (charset[i] == current) { idx = i; break; }
    }
    idx += s.encDelta;
    if (idx < 0) idx = charsetLen - 1;
    if (idx >= charsetLen) idx = 0;
    _editStepName[_editNameCursor] = charset[idx];
    if (_editNameCursor + 1 < STEP_NAME_LEN) {
      _editStepName[_editNameCursor + 1] = '\0';
    }
  }

  // OK moves cursor right or saves if at end
  if (s.okPressed) {
    if (_editNameCursor < STEP_NAME_LEN - 2) {
      _editNameCursor++;
      if (_editStepName[_editNameCursor] == '\0') {
        _editStepName[_editNameCursor] = ' ';
        _editStepName[_editNameCursor + 1] = '\0';
      }
    } else {
      // Trim trailing spaces and save
      int len = strlen(_editStepName);
      while (len > 0 && _editStepName[len - 1] == ' ') len--;
      _editStepName[len] = '\0';
      strncpy(_session->settings().steps[_editStepIdx].name, _editStepName, STEP_NAME_LEN);
      _screen = Screen::StepDetailMenu;
      changed = true;
    }
  }
  
  // Encoder switch saves immediately
  if (s.encSwPressed) {
    int len = strlen(_editStepName);
    while (len > 0 && _editStepName[len - 1] == ' ') len--;
    _editStepName[len] = '\0';
    strncpy(_session->settings().steps[_editStepIdx].name, _editStepName, STEP_NAME_LEN);
    _screen = Screen::StepDetailMenu;
    changed = true;
  }
  
  if (s.backPressed || s.a0BackPressed) {
    // Move cursor left, or exit if at start
    if (_editNameCursor > 0) {
      _editNameCursor--;
    } else {
      _screen = Screen::StepDetailMenu;
    }
  }
  
  return changed;
}

// ===== Step-level TempCoef Override =====
bool MenuController::handleEditStepTempCoefOverride(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editStepTempCoefOverride = !_editStepTempCoefOverride;
  }

  if (s.okPressed || s.encSwPressed) {
    auto& step = _session->settings().steps[_editStepIdx];
    step.tempCoefOverride = _editStepTempCoefOverride;
    if (_editStepTempCoefOverride) {
      // Go to step-specific temp coef settings
      _editTempCoefEnabled = step.tempCoefEnabled;
      _screen = Screen::EditStepTempCoefEnabled;
    } else {
      _screen = Screen::StepDetailMenu;
    }
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::StepDetailMenu;
  }
  
  return changed;
}

bool MenuController::handleEditStepTempCoefEnabled(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editTempCoefEnabled = !_editTempCoefEnabled;
  }

  if (s.okPressed || s.encSwPressed) {
    auto& step = _session->settings().steps[_editStepIdx];
    step.tempCoefEnabled = _editTempCoefEnabled;
    if (_editTempCoefEnabled) {
      _editTempCoefBase = step.tempCoefBase;
      _screen = Screen::EditStepTempCoefBase;
    } else {
      _screen = Screen::StepDetailMenu;
    }
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::StepDetailMenu;
  }
  
  return changed;
}

bool MenuController::handleEditStepTempCoefBase(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editTempCoefBase += s.encDelta * 0.5f;
    if (_editTempCoefBase < 15.0f) _editTempCoefBase = 15.0f;
    if (_editTempCoefBase > 30.0f) _editTempCoefBase = 30.0f;
  }

  if (s.okPressed || s.encSwPressed) {
    auto& step = _session->settings().steps[_editStepIdx];
    step.tempCoefBase = _editTempCoefBase;
    _editTempCoefPercent = step.tempCoefPercent;
    _screen = Screen::EditStepTempCoefPercent;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::StepDetailMenu;
  }
  
  return changed;
}

bool MenuController::handleEditStepTempCoefPercent(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editTempCoefPercent += s.encDelta;
    if (_editTempCoefPercent < 1.0f) _editTempCoefPercent = 1.0f;
    if (_editTempCoefPercent > 50.0f) _editTempCoefPercent = 50.0f;
  }

  if (s.okPressed || s.encSwPressed) {
    auto& step = _session->settings().steps[_editStepIdx];
    step.tempCoefPercent = _editTempCoefPercent;
    _editTempCoefTarget = step.tempCoefTarget;
    _screen = Screen::EditStepTempCoefTarget;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::StepDetailMenu;
  }
  
  return changed;
}

bool MenuController::handleEditStepTempCoefTarget(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    int8_t v = (int8_t)_editTempCoefTarget + s.encDelta;
    if (v < 0) v = 2;
    if (v > 2) v = 0;
    _editTempCoefTarget = (TempCoefTarget)v;
  }

  if (s.okPressed || s.encSwPressed) {
    auto& step = _session->settings().steps[_editStepIdx];
    step.tempCoefTarget = _editTempCoefTarget;
    _editTempAlarmAction = step.tempAlarmAction;
    _screen = Screen::EditStepTempAlarmAction;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::StepDetailMenu;
  }
  
  return changed;
}

bool MenuController::handleEditStepTempAlarmAction(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    int8_t v = (int8_t)_editTempAlarmAction + s.encDelta;
    if (v < 0) v = 3;
    if (v > 3) v = 0;
    _editTempAlarmAction = (TempAlarmAction)v;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().steps[_editStepIdx].tempAlarmAction = _editTempAlarmAction;
    _screen = Screen::StepDetailMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::StepDetailMenu;
  }
  
  return changed;
}

// ===== Reverse Submenu =====
void MenuController::handleReverseMenu(const InputsSnapshot& s) {
  const int8_t ITEMS = 2;  // Enabled, Interval
  
  if (s.encDelta != 0) {
    _subMenuIdx += s.encDelta;
    if (_subMenuIdx < 0) _subMenuIdx = ITEMS - 1;
    if (_subMenuIdx >= ITEMS) _subMenuIdx = 0;
  }

  if (s.okPressed || s.encSwPressed) {
    const auto& set = _session->settings();
    switch (_subMenuIdx) {
      case 0:
        _editReverseEnabled = set.reverseEnabled;
        _screen = Screen::EditReverseEnabled;
        break;
      case 1:
        _editReverseInterval = set.reverseIntervalSec;
        _screen = Screen::EditReverseInterval;
        break;
    }
  }

  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::Menu;
  }
}

bool MenuController::handleEditReverseEnabled(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editReverseEnabled = !_editReverseEnabled;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().reverseEnabled = _editReverseEnabled;
    _screen = Screen::ReverseMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::ReverseMenu;
  }
  
  return changed;
}

bool MenuController::handleEditReverseInterval(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editReverseInterval += s.encDelta;
    if (_editReverseInterval < 1.0f) _editReverseInterval = 1.0f;
    if (_editReverseInterval > 120.0f) _editReverseInterval = 120.0f;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().reverseIntervalSec = _editReverseInterval;
    _screen = Screen::ReverseMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::ReverseMenu;
  }
  
  return changed;
}

// ===== TempCoef (Profile-level, accessed from ProfileSettingsMenu) =====
// handleTempCoefMenu is no longer needed - settings accessed from ProfileSettingsMenu
void MenuController::handleTempCoefMenu(const InputsSnapshot& s) {
  // Redirect to ProfileSettingsMenu (legacy, should not be called)
  _screen = Screen::ProfileSettingsMenu;
}

bool MenuController::handleEditTempCoefEnabled(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editTempCoefEnabled = !_editTempCoefEnabled;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().tempCoefEnabled = _editTempCoefEnabled;
    _screen = Screen::ProfileSettingsMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::ProfileSettingsMenu;
  }
  
  return changed;
}

bool MenuController::handleEditTempCoefBase(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editTempCoefBase += s.encDelta * 0.5f;
    if (_editTempCoefBase < 15.0f) _editTempCoefBase = 15.0f;
    if (_editTempCoefBase > 30.0f) _editTempCoefBase = 30.0f;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().tempCoefBase = _editTempCoefBase;
    _screen = Screen::ProfileSettingsMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::ProfileSettingsMenu;
  }
  
  return changed;
}

bool MenuController::handleEditTempCoefPercent(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editTempCoefPercent += s.encDelta;
    if (_editTempCoefPercent < 1.0f) _editTempCoefPercent = 1.0f;
    if (_editTempCoefPercent > 30.0f) _editTempCoefPercent = 30.0f;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().tempCoefPercent = _editTempCoefPercent;
    _screen = Screen::ProfileSettingsMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::ProfileSettingsMenu;
  }
  
  return changed;
}

bool MenuController::handleEditTempCoefTarget(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    int8_t t = (int8_t)_editTempCoefTarget + s.encDelta;
    if (t < 0) t = 2;
    if (t > 2) t = 0;
    _editTempCoefTarget = (TempCoefTarget)t;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().tempCoefTarget = _editTempCoefTarget;
    _screen = Screen::ProfileSettingsMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::ProfileSettingsMenu;
  }
  
  return changed;
}

bool MenuController::handleEditTempAlarmAction(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    int8_t v = (int8_t)_editTempAlarmAction + s.encDelta;
    if (v < 0) v = 3;
    if (v > 3) v = 0;
    _editTempAlarmAction = (TempAlarmAction)v;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().tempAlarmAction = _editTempAlarmAction;
    _screen = Screen::ProfileSettingsMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::ProfileSettingsMenu;
  }
  
  return changed;
}

// ===== Temp Limits =====
bool MenuController::handleEditTempLimitsEnabled(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editTempLimitsEnabled = !_editTempLimitsEnabled;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().tempLimitsEnabled = _editTempLimitsEnabled;
    _screen = Screen::ProfileSettingsMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::ProfileSettingsMenu;
  }
  
  return changed;
}

bool MenuController::handleEditTempMin(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editTempMin += s.encDelta * 0.5f;
    if (_editTempMin < 10.0f) _editTempMin = 10.0f;
    if (_editTempMin > _editTempMax - 1.0f) _editTempMin = _editTempMax - 1.0f;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().tempMin = _editTempMin;
    _screen = Screen::ProfileSettingsMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::ProfileSettingsMenu;
  }
  
  return changed;
}

bool MenuController::handleEditTempMax(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editTempMax += s.encDelta * 0.5f;
    if (_editTempMax < _editTempMin + 1.0f) _editTempMax = _editTempMin + 1.0f;
    if (_editTempMax > 40.0f) _editTempMax = 40.0f;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().tempMax = _editTempMax;
    _screen = Screen::ProfileSettingsMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::ProfileSettingsMenu;
  }
  
  return changed;
}

// ===== Buzzer Submenu =====
void MenuController::handleBuzzerMenu(const InputsSnapshot& s) {
  const int8_t ITEMS = 6;  // Enabled, StepFinished, ProcessEnded, TempWarning, Freq, Test
  
  if (s.encDelta != 0) {
    _subMenuIdx += s.encDelta;
    if (_subMenuIdx < 0) _subMenuIdx = ITEMS - 1;
    if (_subMenuIdx >= ITEMS) _subMenuIdx = 0;
  }

  if (s.okPressed || s.encSwPressed) {
    switch (_subMenuIdx) {
      case 0:
        _screen = Screen::EditBuzzerEnabled;
        break;
      case 1:
        _screen = Screen::EditBuzzerStepFinished;
        break;
      case 2:
        _screen = Screen::EditBuzzerProcessEnded;
        break;
      case 3:
        _screen = Screen::EditBuzzerTempWarning;
        break;
      case 4:
        _screen = Screen::EditBuzzerFreq;
        break;
      case 5:
        _screen = Screen::BuzzerTest;
        break;
    }
  }

  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::Menu;
  }
}

bool MenuController::handleEditBuzzerEnabled(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editBuzzerEnabled = !_editBuzzerEnabled;
  }

  if (s.okPressed || s.encSwPressed) {
    _screen = Screen::BuzzerMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::BuzzerMenu;
  }
  
  return changed;
}

bool MenuController::handleEditBuzzerStepFinished(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editBuzzerStepFinished = !_editBuzzerStepFinished;
  }

  if (s.okPressed || s.encSwPressed) {
    _screen = Screen::BuzzerMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::BuzzerMenu;
  }
  
  return changed;
}

bool MenuController::handleEditBuzzerProcessEnded(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editBuzzerProcessEnded = !_editBuzzerProcessEnded;
  }

  if (s.okPressed || s.encSwPressed) {
    _screen = Screen::BuzzerMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::BuzzerMenu;
  }
  
  return changed;
}

bool MenuController::handleEditBuzzerTempWarning(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editBuzzerTempWarning = !_editBuzzerTempWarning;
  }

  if (s.okPressed || s.encSwPressed) {
    _screen = Screen::BuzzerMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::BuzzerMenu;
  }
  
  return changed;
}

bool MenuController::handleEditBuzzerFreq(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editBuzzerFreq += s.encDelta * 100;
    if (_editBuzzerFreq < 800) _editBuzzerFreq = 800;
    if (_editBuzzerFreq > 4000) _editBuzzerFreq = 4000;
  }

  if (s.okPressed || s.encSwPressed) {
    _screen = Screen::BuzzerMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::BuzzerMenu;
  }
  
  return changed;
}

void MenuController::handleBuzzerTest(const InputsSnapshot& s) {
  if (s.okPressed || s.encSwPressed) {
    if (_buzzerTestCb) _buzzerTestCb();
  }
  
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::BuzzerMenu;
  }
}

// ===== Hardware Submenu =====
void MenuController::handleHardwareMenu(const InputsSnapshot& s) {
  const int8_t ITEMS = 7;  // StepsPerRev, Microsteps, Driver, Invert, BuzzerType, ActiveHigh, TempOffset
  
  if (s.encDelta != 0) {
    _subMenuIdx += s.encDelta;
    if (_subMenuIdx < 0) _subMenuIdx = ITEMS - 1;
    if (_subMenuIdx >= ITEMS) _subMenuIdx = 0;
  }

  if (s.okPressed || s.encSwPressed) {
    switch (_subMenuIdx) {
      case 0:
        _editStepsPerRev = _hwSettings.stepsPerRev;
        _screen = Screen::EditStepsPerRev;
        break;
      case 1:
        _editMicrosteps = _hwSettings.microsteps;
        _screen = Screen::EditMicrosteps;
        break;
      case 2:
        _editDriverType = _hwSettings.driverType;
        _screen = Screen::EditDriverType;
        break;
      case 3:
        _editMotorInvert = _hwSettings.motorInvertDir;
        _screen = Screen::EditMotorInvert;
        break;
      case 4:
        _editBuzzerType = _hwSettings.buzzerType;
        _screen = Screen::EditBuzzerType;
        break;
      case 5:
        _editBuzzerActiveHigh = _hwSettings.buzzerActiveHigh;
        _screen = Screen::EditBuzzerActiveHigh;
        break;
      case 6:
        _editTempOffset = _hwSettings.tempOffset;
        _screen = Screen::EditTempOffset;
        break;
    }
  }

  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::Menu;
  }
}

bool MenuController::handleEditStepsPerRev(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    // Toggle between common values: 200, 400
    if (_editStepsPerRev == 200) _editStepsPerRev = 400;
    else _editStepsPerRev = 200;
  }

  if (s.okPressed || s.encSwPressed) {
    _hwSettings.stepsPerRev = _editStepsPerRev;
    _screen = Screen::HardwareMenu;
    changed = true;
    if (_hwChangedCb) _hwChangedCb();
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::HardwareMenu;
  }
  
  return changed;
}

bool MenuController::handleEditMicrosteps(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    // Cycle through: 1, 2, 4, 8, 16, 32
    static const uint8_t vals[] = {1, 2, 4, 8, 16, 32};
    int8_t idx = 0;
    for (int8_t i = 0; i < 6; i++) {
      if (vals[i] == _editMicrosteps) { idx = i; break; }
    }
    idx += s.encDelta;
    if (idx < 0) idx = 5;
    if (idx > 5) idx = 0;
    _editMicrosteps = vals[idx];
  }

  if (s.okPressed || s.encSwPressed) {
    _hwSettings.microsteps = _editMicrosteps;
    _screen = Screen::HardwareMenu;
    changed = true;
    if (_hwChangedCb) _hwChangedCb();
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::HardwareMenu;
  }
  
  return changed;
}

bool MenuController::handleEditDriverType(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editDriverType = (_editDriverType == DriverType::Generic) 
                      ? DriverType::TMC2209 : DriverType::Generic;
  }

  if (s.okPressed || s.encSwPressed) {
    _hwSettings.driverType = _editDriverType;
    _screen = Screen::HardwareMenu;
    changed = true;
    if (_hwChangedCb) _hwChangedCb();
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::HardwareMenu;
  }
  
  return changed;
}

bool MenuController::handleEditMotorInvert(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editMotorInvert = !_editMotorInvert;
  }

  if (s.okPressed || s.encSwPressed) {
    _hwSettings.motorInvertDir = _editMotorInvert;
    _screen = Screen::HardwareMenu;
    changed = true;
    if (_hwChangedCb) _hwChangedCb();
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::HardwareMenu;
  }
  
  return changed;
}

bool MenuController::handleEditBuzzerType(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editBuzzerType = (_editBuzzerType == BuzzerType::Active) 
                      ? BuzzerType::Passive : BuzzerType::Active;
  }

  if (s.okPressed || s.encSwPressed) {
    _hwSettings.buzzerType = _editBuzzerType;
    _screen = Screen::HardwareMenu;
    changed = true;
    if (_hwChangedCb) _hwChangedCb();
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::HardwareMenu;
  }
  
  return changed;
}

bool MenuController::handleEditBuzzerActiveHigh(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editBuzzerActiveHigh = !_editBuzzerActiveHigh;
  }

  if (s.okPressed || s.encSwPressed) {
    _hwSettings.buzzerActiveHigh = _editBuzzerActiveHigh;
    _screen = Screen::HardwareMenu;
    changed = true;
    if (_hwChangedCb) _hwChangedCb();
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::HardwareMenu;
  }
  
  return changed;
}

bool MenuController::handleEditTempOffset(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editTempOffset += s.encDelta * 0.1f;
    if (_editTempOffset < -5.0f) _editTempOffset = -5.0f;
    if (_editTempOffset > 5.0f) _editTempOffset = 5.0f;
  }

  if (s.okPressed || s.encSwPressed) {
    _hwSettings.tempOffset = _editTempOffset;
    _screen = Screen::HardwareMenu;
    changed = true;
    if (_hwChangedCb) _hwChangedCb();
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::HardwareMenu;
  }
  
  return changed;
}
