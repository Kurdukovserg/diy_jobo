#include "MenuController.h"
#include "Buzzer.h"

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
    case Screen::EditRpm:
      settingsChanged = handleEditRpm(s);
      break;
    // Steps submenu
    case Screen::StepsMenu:
      handleStepsMenu(s);
      break;
    case Screen::EditStepDuration:
      settingsChanged = handleEditStepDuration(s);
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
    // TempCoef submenu
    case Screen::TempCoefMenu:
      handleTempCoefMenu(s);
      break;
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
    // Buzzer submenu
    case Screen::BuzzerMenu:
      handleBuzzerMenu(s);
      break;
    case Screen::EditBuzzerEnabled:
      settingsChanged = handleEditBuzzerEnabled(s);
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
        _editRpm = _session->settings().targetRpm;
        _screen = Screen::EditRpm; 
        break;
      case 1: 
        _screen = Screen::StepsMenu; 
        break;
      case 2: 
        _screen = Screen::ReverseMenu; 
        break;
      case 3: 
        _screen = Screen::TempCoefMenu; 
        break;
      case 4:
        _screen = Screen::BuzzerMenu;
        break;
    }
  }

  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::Main;
  }
}

bool MenuController::handleEditRpm(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editRpm += s.encDelta;
    if (_editRpm < RPM_MIN) _editRpm = RPM_MIN;
    if (_editRpm > RPM_MAX) _editRpm = RPM_MAX;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().targetRpm = _editRpm;
    _screen = Screen::Menu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::Menu;
  }
  
  return changed;
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
      // Edit existing step
      _editStepIdx = _subMenuIdx;
      _editStepDuration = set.steps[_editStepIdx].durationSec;
      _screen = Screen::EditStepDuration;
    } else if (set.stepCount < MAX_STEPS) {
      // Add new step
      set.steps[set.stepCount].durationSec = 60;  // default 1 min
      set.stepCount++;
      _editStepIdx = set.stepCount - 1;
      _editStepDuration = set.steps[_editStepIdx].durationSec;
      _screen = Screen::EditStepDuration;
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
    _screen = Screen::Menu;
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
    _screen = Screen::StepsMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::StepsMenu;
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

// ===== TempCoef Submenu =====
void MenuController::handleTempCoefMenu(const InputsSnapshot& s) {
  const int8_t ITEMS = 7;  // Enabled, Base, Percent, Target, LimitsEnabled, Min, Max
  
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
        _editTempLimitsEnabled = set.tempLimitsEnabled;
        _screen = Screen::EditTempLimitsEnabled;
        break;
      case 5:
        _editTempMin = set.tempMin;
        _screen = Screen::EditTempMin;
        break;
      case 6:
        _editTempMax = set.tempMax;
        _screen = Screen::EditTempMax;
        break;
    }
  }

  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::Menu;
  }
}

bool MenuController::handleEditTempCoefEnabled(const InputsSnapshot& s) {
  bool changed = false;
  
  if (s.encDelta != 0) {
    _editTempCoefEnabled = !_editTempCoefEnabled;
  }

  if (s.okPressed || s.encSwPressed) {
    _session->settings().tempCoefEnabled = _editTempCoefEnabled;
    _screen = Screen::TempCoefMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::TempCoefMenu;
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
    _screen = Screen::TempCoefMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::TempCoefMenu;
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
    _screen = Screen::TempCoefMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::TempCoefMenu;
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
    _screen = Screen::TempCoefMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::TempCoefMenu;
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
    _screen = Screen::TempCoefMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::TempCoefMenu;
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
    _screen = Screen::TempCoefMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::TempCoefMenu;
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
    _screen = Screen::TempCoefMenu;
    changed = true;
  }
  if (s.backPressed || s.a0BackPressed) {
    _screen = Screen::TempCoefMenu;
  }
  
  return changed;
}

// ===== Buzzer Submenu =====
void MenuController::handleBuzzerMenu(const InputsSnapshot& s) {
  const int8_t ITEMS = 2;  // Enabled, Test
  
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
        // Test beep
        buzzer.beep(1000, 200);
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
    buzzer.enabled = !buzzer.enabled;
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
