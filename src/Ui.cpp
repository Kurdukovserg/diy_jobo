#include "Ui.h"
#include "Config.h"
#include "MenuController.h"
#include "SessionController.h"
#include <cstdio>

void Ui::begin() {
  _u8g2.begin();
}

void Ui::tick(const UiModel& m) {
  if (millis() - _lastDraw < UI_FPS_MS) return;
  _lastDraw = millis();
  draw(m);
}

void Ui::draw(const UiModel& m) {
  _u8g2.clearBuffer();

  switch (m.screen) {
    case Screen::Main:      drawMain(m); break;
    case Screen::Menu:      drawMenu(m); break;
    // Profile
    case Screen::ProfileMenu: drawProfileMenu(m); break;
    case Screen::ProfileEditMenu: drawProfileEditMenu(m); break;
    case Screen::ProfileSettingsMenu: drawProfileSettingsMenu(m); break;
    case Screen::StepsMenu: drawStepsMenu(m); break;
    case Screen::StepDetailMenu: drawStepDetailMenu(m); break;
    case Screen::EditStepDuration: drawEditStepDuration(m); break;
    case Screen::EditStepRpm: drawEditStepRpm(m); break;
    case Screen::EditStepTempMode: drawEditStepTempMode(m); break;
    case Screen::EditStepTempTarget: drawEditStepTempTarget(m); break;
    case Screen::EditStepTempBiasMode: drawEditStepTempBiasMode(m); break;
    case Screen::EditStepTempBias: drawEditStepTempBias(m); break;
    case Screen::EditStepName: drawEditStepName(m); break;
    case Screen::EditStepTempCoefOverride: drawEditStepTempCoefOverride(m); break;
    case Screen::EditStepTempCoefEnabled: drawEditTempCoefEnabled(m); break;
    case Screen::EditStepTempCoefBase: drawEditTempCoefBase(m); break;
    case Screen::EditStepTempCoefPercent: drawEditTempCoefPercent(m); break;
    case Screen::EditStepTempCoefTarget: drawEditTempCoefTarget(m); break;
    case Screen::EditStepTempAlarmAction: drawEditTempAlarmAction(m); break;
    // Reverse
    case Screen::ReverseMenu: drawReverseMenu(m); break;
    case Screen::EditReverseEnabled: drawEditReverseEnabled(m); break;
    case Screen::EditReverseInterval: drawEditReverseInterval(m); break;
    // TempCoef (accessed from ProfileSettingsMenu)
    case Screen::EditTempCoefEnabled: drawEditTempCoefEnabled(m); break;
    case Screen::EditTempCoefBase: drawEditTempCoefBase(m); break;
    case Screen::EditTempCoefPercent: drawEditTempCoefPercent(m); break;
    case Screen::EditTempCoefTarget: drawEditTempCoefTarget(m); break;
    case Screen::EditTempAlarmAction: drawEditTempAlarmAction(m); break;
    // Temp limits
    case Screen::EditTempLimitsEnabled: drawEditTempLimitsEnabled(m); break;
    case Screen::EditTempMin: drawEditTempMin(m); break;
    case Screen::EditTempMax: drawEditTempMax(m); break;
    // Buzzer
    case Screen::BuzzerMenu: drawBuzzerMenu(m); break;
    case Screen::EditBuzzerEnabled: drawEditBuzzerEnabled(m); break;
    case Screen::EditBuzzerStepFinished: drawEditBuzzerStepFinished(m); break;
    case Screen::EditBuzzerProcessEnded: drawEditBuzzerProcessEnded(m); break;
    case Screen::EditBuzzerTempWarning: drawEditBuzzerTempWarning(m); break;
    case Screen::EditBuzzerFreq: drawEditBuzzerFreq(m); break;
    case Screen::BuzzerTest: drawBuzzerTest(m); break;
    // Hardware
    case Screen::HardwareMenu: drawHardwareMenu(m); break;
    case Screen::EditStepsPerRev: drawEditStepsPerRev(m); break;
    case Screen::EditMicrosteps: drawEditMicrosteps(m); break;
    case Screen::EditDriverType: drawEditDriverType(m); break;
    case Screen::EditMotorInvert: drawEditMotorInvert(m); break;
    case Screen::EditBuzzerType: drawEditBuzzerType(m); break;
    case Screen::EditBuzzerActiveHigh: drawEditBuzzerActiveHigh(m); break;
    case Screen::EditTempOffset: drawEditTempOffset(m); break;
  }

  _u8g2.sendBuffer();
}

void Ui::drawMain(const UiModel& m) {
  const int x = 2;

  // Header with profile name or "DIY JOBO"
  _u8g2.setFont(u8g2_font_5x8_tf);
  if (m.profileName[0] != '\0') {
    _u8g2.drawStr(x, 9, m.profileName);
  } else {
    _u8g2.drawStr(x, 9, "DIY JOBO");
  }
  
  char statusBuf[16];
  if (m.run) {
    snprintf(statusBuf, sizeof(statusBuf), "%s%s", 
             m.dirFwd ? "FWD" : "REV",
             m.reverseEnabled ? "~" : "");
  } else {
    snprintf(statusBuf, sizeof(statusBuf), "STOP");
  }
  _u8g2.drawStr(100, 9, statusBuf);
  _u8g2.drawHLine(x, 11, 124);

  // BIG STEP INDICATOR - center of screen
  if (m.stepCount > 0 && m.totalRemainingSec > 0) {
    int mins = m.stepRemainingSec / 60;
    int secs = m.stepRemainingSec % 60;
    
    // Step name or "Step N"
    _u8g2.setFont(u8g2_font_6x13_tf);
    char stepNameBuf[24];
    if (m.currentStepName[0] != '\0') {
      snprintf(stepNameBuf, sizeof(stepNameBuf), "%s", m.currentStepName);
    } else {
      snprintf(stepNameBuf, sizeof(stepNameBuf), "Step %d", m.currentStep + 1);
    }
    int nameW = _u8g2.getStrWidth(stepNameBuf);
    _u8g2.drawStr((128 - nameW) / 2, 24, stepNameBuf);
    
    // Big timer display
    _u8g2.setFont(u8g2_font_logisoso18_tf);
    char timeBuf[12];
    if (m.isPaused) {
      snprintf(timeBuf, sizeof(timeBuf), "DONE");
    } else {
      snprintf(timeBuf, sizeof(timeBuf), "%d:%02d", mins, secs);
    }
    int timeW = _u8g2.getStrWidth(timeBuf);
    _u8g2.drawStr((128 - timeW) / 2, 46, timeBuf);
    
    // RPM and Temp on bottom - compact
    _u8g2.setFont(u8g2_font_5x8_tf);
    char infoBuf[32];
    if (!m.hasTemp || isnan(m.tempC)) {
      snprintf(infoBuf, sizeof(infoBuf), "R:%ld T:--", (long)m.currentStepRpm);
    } else {
      snprintf(infoBuf, sizeof(infoBuf), "R:%ld T:%.1f%s", (long)m.currentStepRpm, m.tempC, 
               m.tempAlarm ? (m.tempLow ? "!" : "!") : "");
    }
    _u8g2.drawStr(x, 56, infoBuf);
    
    // Step progress indicator on right
    char progBuf[8];
    snprintf(progBuf, sizeof(progBuf), "%d/%d", m.currentStep + 1, m.stepCount);
    _u8g2.drawStr(100, 56, progBuf);
  } else {
    // Not running - show RPM and temp in standard layout
    _u8g2.setFont(u8g2_font_6x13_tf);
    char buf[32];
    snprintf(buf, sizeof(buf), "RPM: %ld (%.0f)", (long)m.rpm, m.currentRpm);
    _u8g2.drawStr(x, 26, buf);

    // Temperature
    char tbuf[32];
    if (!m.hasTemp || isnan(m.tempC)) {
      snprintf(tbuf, sizeof(tbuf), "T: --.-C%s", m.tempCoefEnabled ? " *" : "");
    } else {
      const char* alarmStr = "";
      if (m.tempAlarm) alarmStr = m.tempLow ? " LOW!" : " HIGH!";
      snprintf(tbuf, sizeof(tbuf), "T: %.1fC%s%s", m.tempC, m.tempCoefEnabled ? "*" : "", alarmStr);
    }
    _u8g2.drawStr(x, 40, tbuf);

    // Steps count
    _u8g2.setFont(u8g2_font_5x8_tf);
    char stepBuf[24];
    snprintf(stepBuf, sizeof(stepBuf), "Steps: %d", m.stepCount);
    _u8g2.drawStr(x, 52, stepBuf);
  }

  // Hint line
  _u8g2.setFont(u8g2_font_5x8_tf);
  if (m.isPaused) {
    _u8g2.drawStr(x, 63, "OK:next  BACK:stop");
  } else {
    _u8g2.drawStr(x, 63, "OK:run ENC:menu BACK:stop");
  }
}

void Ui::drawMenu(const UiModel& m) {
  const int x = 2;
  const int8_t TOTAL = 4;  // Profile, Reverse, Buzzer, Hardware
  
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "SETTINGS");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_5x8_tf);
  const char* items[] = {"Profile", "Reverse", "Buzzer", "Hardware"};
  char buf[32];
  
  int8_t startY = 26;
  int8_t lineH = 10;
  int8_t visible = 4;
  int8_t scroll = 0;
  if (m.menuIdx >= visible) scroll = m.menuIdx - visible + 1;
  
  for (int8_t i = 0; i < visible && (scroll + i) < TOTAL; i++) {
    int8_t idx = scroll + i;
    int y = startY + i * lineH;
    
    if (idx == m.menuIdx) {
      _u8g2.drawBox(0, y - 8, 128, lineH);
      _u8g2.setDrawColor(0);
    }
    
    switch (idx) {
      case 0: snprintf(buf, sizeof(buf), "%s (%d) >", items[idx], m.stepCount); break;
      default: snprintf(buf, sizeof(buf), "%s >", items[idx]); break;
    }
    _u8g2.drawStr(x, y, buf);
    _u8g2.setDrawColor(1);
  }

  _u8g2.drawStr(x, 63, "ENC:sel  OK:edit  BACK:exit");
}

// ===== Profile Submenu =====
void Ui::drawProfileMenu(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "PROFILE");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_5x8_tf);
  // Future: will have Default, Custom, Profile1, etc.
  const char* items[] = {"Custom"};
  
  for (int i = 0; i < 1; i++) {
    int y = 30 + i * 14;
    bool sel = (i == m.subMenuIdx);
    if (sel) {
      _u8g2.drawBox(0, y - 10, 128, 13);
      _u8g2.setDrawColor(0);
    }
    char buf[24];
    snprintf(buf, sizeof(buf), "%s >", items[i]);
    _u8g2.drawStr(x, y, buf);
    _u8g2.setDrawColor(1);
  }
  _u8g2.drawStr(x, 63, "OK:select  BACK:menu");
}

void Ui::drawProfileEditMenu(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "CUSTOM PROFILE");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_5x8_tf);
  const char* items[] = {"Settings", "Steps"};
  
  for (int i = 0; i < 2; i++) {
    int y = 30 + i * 14;
    bool sel = (i == m.subMenuIdx);
    if (sel) {
      _u8g2.drawBox(0, y - 10, 128, 13);
      _u8g2.setDrawColor(0);
    }
    char buf[24];
    if (i == 1) {
      snprintf(buf, sizeof(buf), "%s (%d) >", items[i], m.stepCount);
    } else {
      snprintf(buf, sizeof(buf), "%s >", items[i]);
    }
    _u8g2.drawStr(x, y, buf);
    _u8g2.setDrawColor(1);
  }
  _u8g2.drawStr(x, 63, "OK:select  BACK:profiles");
}

void Ui::drawProfileSettingsMenu(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "PROFILE SETTINGS");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_5x8_tf);
  const char* items[] = {"TempCoef", "Base", "Percent", "Target", "Alarm", "Limits", "Min", "Max"};
  char buf[32];
  
  const int8_t VISIBLE = 4;
  const int8_t TOTAL = 8;
  int8_t scroll = 0;
  if (m.subMenuIdx >= VISIBLE) scroll = m.subMenuIdx - VISIBLE + 1;
  
  for (int8_t i = 0; i < VISIBLE && (scroll + i) < TOTAL; i++) {
    int8_t idx = scroll + i;
    int y = 24 + i * 10;
    
    if (idx == m.subMenuIdx) {
      _u8g2.drawBox(0, y - 8, 128, 10);
      _u8g2.setDrawColor(0);
    }
    
    switch (idx) {
      case 0: snprintf(buf, sizeof(buf), "%s: %s", items[idx], m.tempCoefEnabled ? "ON" : "OFF"); break;
      case 1: snprintf(buf, sizeof(buf), "%s: %.1fC", items[idx], m.tempCoefBase); break;
      case 2: snprintf(buf, sizeof(buf), "%s: %.0f%%", items[idx], m.tempCoefPercent); break;
      case 3: {
        const char* tgt = "Timer";
        if (m.tempCoefTarget == TempCoefTarget::Rpm) tgt = "RPM";
        else if (m.tempCoefTarget == TempCoefTarget::Both) tgt = "Both";
        snprintf(buf, sizeof(buf), "%s: %s", items[idx], tgt);
        break;
      }
      case 4: {
        const char* act = "None";
        if (m.tempAlarmAction == 1) act = "Beep";
        else if (m.tempAlarmAction == 2) act = "Pause";
        else if (m.tempAlarmAction == 3) act = "Stop";
        snprintf(buf, sizeof(buf), "%s: %s", items[idx], act);
        break;
      }
      case 5: snprintf(buf, sizeof(buf), "%s: %s", items[idx], m.tempLimitsEnabled ? "ON" : "OFF"); break;
      case 6: snprintf(buf, sizeof(buf), "%s: %.1fC", items[idx], m.tempMin); break;
      case 7: snprintf(buf, sizeof(buf), "%s: %.1fC", items[idx], m.tempMax); break;
    }
    _u8g2.drawStr(x, y, buf);
    _u8g2.setDrawColor(1);
  }

  _u8g2.drawStr(x, 63, "OK:edit  BACK:profile");
}

// ===== Steps Submenu =====
void Ui::drawStepsMenu(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "DEV STEPS");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_5x8_tf);
  char buf[32];
  
  // Show up to 4 visible items with scrolling
  const int VISIBLE = 4;
  int itemCount = m.stepCount + (m.stepCount < 10 ? 1 : 0);  // +1 for "Add"
  int scrollOff = 0;
  if (m.subMenuIdx >= VISIBLE) scrollOff = m.subMenuIdx - VISIBLE + 1;
  
  for (int v = 0; v < VISIBLE && (v + scrollOff) < itemCount; v++) {
    int i = v + scrollOff;
    int y = 24 + v * 10;
    bool sel = (i == m.subMenuIdx);
    
    if (sel) {
      _u8g2.drawBox(0, y - 7, 128, 10);
      _u8g2.setDrawColor(0);
    }
    
    if (i < m.stepCount) {
      // Existing step - show duration from stepDurations array
      int32_t dur = m.stepDurations[i];
      int mins = dur / 60;
      int secs = dur % 60;
      snprintf(buf, sizeof(buf), "Step %d: %d:%02d", i + 1, mins, secs);
    } else {
      snprintf(buf, sizeof(buf), "+ Add Step");
    }
    _u8g2.drawStr(x, y, buf);
    _u8g2.setDrawColor(1);
  }
  
  // Scroll indicators
  if (scrollOff > 0) _u8g2.drawStr(120, 20, "^");
  if (scrollOff + VISIBLE < itemCount) _u8g2.drawStr(120, 54, "v");
  
  _u8g2.drawStr(x, 63, "OK:edit LONG:del BACK:menu");
}

void Ui::drawEditStepDuration(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  char title[24];
  snprintf(title, sizeof(title), "STEP %d DURATION", m.editStepIdx + 1);
  _u8g2.drawStr(x, 12, title);
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  char buf[16];
  if (m.editStepDuration > 0) {
    int mins = m.editStepDuration / 60;
    int secs = m.editStepDuration % 60;
    snprintf(buf, sizeof(buf), "%d:%02d", mins, secs);
  } else {
    snprintf(buf, sizeof(buf), "0:00");
  }
  int w = _u8g2.getStrWidth(buf);
  _u8g2.drawStr((128 - w) / 2, 48, buf);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:+/-10s  OK:save  BACK:cancel");
}

void Ui::drawStepDetailMenu(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  char title[24];
  snprintf(title, sizeof(title), "STEP %d", m.editStepIdx + 1);
  _u8g2.drawStr(x, 12, title);
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_5x8_tf);
  const char* items[] = {"Duration", "RPM", "Temp", "Bias", "Name", "TempCoef"};
  char buf[32];
  
  const int8_t TOTAL = 6;
  const int8_t VISIBLE = 4;
  int8_t scroll = 0;
  if (m.editStepDetailIdx >= VISIBLE) scroll = m.editStepDetailIdx - VISIBLE + 1;
  
  for (int8_t v = 0; v < VISIBLE && (scroll + v) < TOTAL; v++) {
    int8_t i = scroll + v;
    int y = 24 + v * 10;
    bool sel = (i == m.editStepDetailIdx);
    if (sel) {
      _u8g2.drawBox(0, y - 7, 128, 10);
      _u8g2.setDrawColor(0);
    }
    
    switch (i) {
      case 0: {
        int32_t dur = m.stepDurations[m.editStepIdx];
        snprintf(buf, sizeof(buf), "%s: %d:%02d", items[i], (int)(dur/60), (int)(dur%60));
        break;
      }
      case 1:
        snprintf(buf, sizeof(buf), "%s: %ld", items[i], (long)m.editStepRpm);
        break;
      case 2:
        if (m.editStepTempMode == 0) {
          snprintf(buf, sizeof(buf), "%s: OFF", items[i]);
        } else {
          snprintf(buf, sizeof(buf), "%s: %.1fC", items[i], m.editStepTempTarget);
        }
        break;
      case 3:
        if (m.editStepTempBiasMode == 0) {
          snprintf(buf, sizeof(buf), "%s: OFF", items[i]);
        } else {
          snprintf(buf, sizeof(buf), "%s: +/-%.1fC", items[i], m.editStepTempBias);
        }
        break;
      case 4:
        if (m.editStepName[0] != '\0') {
          snprintf(buf, sizeof(buf), "%s: %s", items[i], m.editStepName);
        } else {
          snprintf(buf, sizeof(buf), "%s: (none)", items[i]);
        }
        break;
      case 5:
        snprintf(buf, sizeof(buf), "%s: %s >", items[i], m.editStepTempCoefOverride ? "Custom" : "Profile");
        break;
    }
    _u8g2.drawStr(x, y, buf);
    _u8g2.setDrawColor(1);
  }

  _u8g2.drawStr(x, 63, "OK:edit  BACK:steps");
}

void Ui::drawEditStepRpm(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  char title[24];
  snprintf(title, sizeof(title), "STEP %d RPM", m.editStepIdx + 1);
  _u8g2.drawStr(x, 12, title);
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  char buf[8];
  snprintf(buf, sizeof(buf), "%ld", (long)m.editStepRpm);
  int w = _u8g2.getStrWidth(buf);
  _u8g2.drawStr((128 - w) / 2, 48, buf);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:adj  OK:save  BACK:cancel");
}

void Ui::drawEditStepTempMode(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  char title[24];
  snprintf(title, sizeof(title), "STEP %d TEMP", m.editStepIdx + 1);
  _u8g2.drawStr(x, 12, title);
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = (m.editStepTempMode == 0) ? "OFF" : "ON";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:toggle  OK:save  BACK:cancel");
}

void Ui::drawEditStepTempTarget(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  char title[24];
  snprintf(title, sizeof(title), "STEP %d TARGET", m.editStepIdx + 1);
  _u8g2.drawStr(x, 12, title);
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f", m.editStepTempTarget);
  int w = _u8g2.getStrWidth(buf);
  _u8g2.drawStr((128 - w) / 2, 48, buf);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:+/-0.5  OK:save  BACK:cancel");
}

void Ui::drawEditStepTempBiasMode(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  char title[24];
  snprintf(title, sizeof(title), "STEP %d BIAS", m.editStepIdx + 1);
  _u8g2.drawStr(x, 12, title);
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = (m.editStepTempBiasMode == 0) ? "OFF" : "ON";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:toggle  OK:save  BACK:cancel");
}

void Ui::drawEditStepTempBias(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  char title[24];
  snprintf(title, sizeof(title), "STEP %d +/-", m.editStepIdx + 1);
  _u8g2.drawStr(x, 12, title);
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f", m.editStepTempBias);
  int w = _u8g2.getStrWidth(buf);
  _u8g2.drawStr((128 - w) / 2, 48, buf);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:+/-0.5  OK:save  BACK:cancel");
}

void Ui::drawEditStepName(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  char title[24];
  snprintf(title, sizeof(title), "STEP %d NAME", m.editStepIdx + 1);
  _u8g2.drawStr(x, 12, title);
  _u8g2.drawHLine(x, 14, 124);

  // Show name with cursor
  _u8g2.setFont(u8g2_font_10x20_tf);
  char nameBuf[16];
  strncpy(nameBuf, m.editStepName, sizeof(nameBuf) - 1);
  nameBuf[sizeof(nameBuf) - 1] = '\0';
  int w = _u8g2.getStrWidth(nameBuf);
  int nameX = (128 - w) / 2;
  _u8g2.drawStr(nameX, 40, nameBuf);
  
  // Draw cursor underline
  int cursorX = nameX + m.editNameCursor * 10;  // approximate char width
  _u8g2.drawHLine(cursorX, 42, 9);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:char OK:next BACK:prev/exit");
}

void Ui::drawEditStepTempCoefOverride(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  char title[24];
  snprintf(title, sizeof(title), "STEP %d TEMPCOEF", m.editStepIdx + 1);
  _u8g2.drawStr(x, 12, title);
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = m.editStepTempCoefOverride ? "Custom" : "Profile";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:toggle  OK:save  BACK:cancel");
}

void Ui::drawEditTempAlarmAction(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "ALARM ACTION");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = "None";
  if (m.tempAlarmAction == 1) val = "Beep";
  else if (m.tempAlarmAction == 2) val = "Pause";
  else if (m.tempAlarmAction == 3) val = "Stop";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:select  OK:save  BACK:cancel");
}

// ===== Reverse Submenu =====
void Ui::drawReverseMenu(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "AUTO-REVERSE");
  _u8g2.drawHLine(x, 14, 124);

  const char* items[] = {"Enabled", "Interval"};
  char buf[32];
  
  for (int i = 0; i < 2; i++) {
    int y = 30 + i * 14;
    bool sel = (i == m.subMenuIdx);
    if (sel) {
      _u8g2.drawBox(0, y - 10, 128, 13);
      _u8g2.setDrawColor(0);
    }
    if (i == 0) {
      snprintf(buf, sizeof(buf), "%s: %s", items[i], m.reverseEnabled ? "ON" : "OFF");
    } else {
      snprintf(buf, sizeof(buf), "%s: %.0fs", items[i], m.reverseIntervalSec);
    }
    _u8g2.drawStr(x, y, buf);
    _u8g2.setDrawColor(1);
  }
  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "OK:edit  BACK:menu");
}

void Ui::drawEditReverseEnabled(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "AUTO-REVERSE");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = m.reverseEnabled ? "ON" : "OFF";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:toggle  OK:save  BACK:cancel");
}

void Ui::drawEditReverseInterval(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "REVERSE INTERVAL");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  char buf[16];
  snprintf(buf, sizeof(buf), "%.0fs", m.reverseIntervalSec);
  int w = _u8g2.getStrWidth(buf);
  _u8g2.drawStr((128 - w) / 2, 48, buf);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:adj  OK:save  BACK:cancel");
}

// ===== TempCoef Submenu =====
void Ui::drawTempCoefMenu(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "TEMPERATURE");
  _u8g2.drawHLine(x, 14, 124);

  char buf[32];
  const int VISIBLE = 4;
  const int ITEMS = 7;  // Enabled, Base, Percent, Target, LimitsEnabled, Min, Max
  int scrollOff = 0;
  if (m.subMenuIdx >= VISIBLE) scrollOff = m.subMenuIdx - VISIBLE + 1;
  
  for (int v = 0; v < VISIBLE && (v + scrollOff) < ITEMS; v++) {
    int i = v + scrollOff;
    int y = 26 + v * 10;
    bool sel = (i == m.subMenuIdx);
    _u8g2.setFont(u8g2_font_5x8_tf);
    if (sel) {
      _u8g2.drawBox(0, y - 7, 128, 10);
      _u8g2.setDrawColor(0);
    }
    switch (i) {
      case 0: snprintf(buf, sizeof(buf), "Coef: %s", m.tempCoefEnabled ? "ON" : "OFF"); break;
      case 1: snprintf(buf, sizeof(buf), "Base: %.1fC", m.tempCoefBase); break;
      case 2: snprintf(buf, sizeof(buf), "Percent: %.0f%%", m.tempCoefPercent); break;
      case 3: {
        const char* tgt = "Timer";
        if (m.tempCoefTarget == TempCoefTarget::Rpm) tgt = "RPM";
        else if (m.tempCoefTarget == TempCoefTarget::Both) tgt = "Both";
        snprintf(buf, sizeof(buf), "Apply: %s", tgt);
        break;
      }
      case 4: snprintf(buf, sizeof(buf), "Limits: %s", m.tempLimitsEnabled ? "ON" : "OFF"); break;
      case 5: snprintf(buf, sizeof(buf), "Min: %.1fC", m.tempMin); break;
      case 6: snprintf(buf, sizeof(buf), "Max: %.1fC", m.tempMax); break;
    }
    _u8g2.drawStr(x, y, buf);
    _u8g2.setDrawColor(1);
  }
  
  // Scroll indicators
  _u8g2.setFont(u8g2_font_5x8_tf);
  if (scrollOff > 0) _u8g2.drawStr(120, 20, "^");
  if (scrollOff + VISIBLE < ITEMS) _u8g2.drawStr(120, 54, "v");
  _u8g2.drawStr(x, 63, "OK:edit  BACK:menu");
}

void Ui::drawEditTempCoefEnabled(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "TEMP COEF ENABLED");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = m.tempCoefEnabled ? "ON" : "OFF";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:toggle  OK:save  BACK:cancel");
}

void Ui::drawEditTempCoefBase(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "BASE TEMPERATURE");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f", m.tempCoefBase);
  int w = _u8g2.getStrWidth(buf);
  _u8g2.drawStr((128 - w) / 2, 48, buf);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:+/-0.5  OK:save  BACK:cancel");
}

void Ui::drawEditTempCoefPercent(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "COEF PERCENT");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  char buf[16];
  snprintf(buf, sizeof(buf), "%.0f%%", m.tempCoefPercent);
  int w = _u8g2.getStrWidth(buf);
  _u8g2.drawStr((128 - w) / 2, 48, buf);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:adj  OK:save  BACK:cancel");
}

void Ui::drawEditTempCoefTarget(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "APPLY COEF TO");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = "Timer";
  if (m.tempCoefTarget == TempCoefTarget::Rpm) val = "RPM";
  else if (m.tempCoefTarget == TempCoefTarget::Both) val = "Both";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:cycle  OK:save  BACK:cancel");
}

// ===== Temp Limits =====
void Ui::drawEditTempLimitsEnabled(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "TEMP LIMITS");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = m.tempLimitsEnabled ? "ON" : "OFF";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:toggle  OK:save  BACK:cancel");
}

void Ui::drawEditTempMin(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "MIN TEMPERATURE");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f", m.tempMin);
  int w = _u8g2.getStrWidth(buf);
  _u8g2.drawStr((128 - w) / 2, 48, buf);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:+/-0.5  OK:save  BACK:cancel");
}

void Ui::drawEditTempMax(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "MAX TEMPERATURE");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f", m.tempMax);
  int w = _u8g2.getStrWidth(buf);
  _u8g2.drawStr((128 - w) / 2, 48, buf);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:+/-0.5  OK:save  BACK:cancel");
}

// ===== Buzzer Menu =====
void Ui::drawBuzzerMenu(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "BUZZER");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_5x8_tf);
  const char* items[] = {
    "Enabled", "Step done", "Process end", "Temp warn", "Frequency", "Test"
  };
  const int8_t itemCount = 6;
  
  int8_t startY = 26;
  int8_t lineH = 10;
  int8_t visible = 4;
  int8_t scroll = 0;
  if (m.subMenuIdx >= visible) scroll = m.subMenuIdx - visible + 1;
  
  for (int8_t i = 0; i < visible && (scroll + i) < itemCount; i++) {
    int8_t idx = scroll + i;
    int y = startY + i * lineH;
    if (idx == m.subMenuIdx) {
      _u8g2.drawBox(0, y - 8, 128, lineH);
      _u8g2.setDrawColor(0);
    }
    _u8g2.drawStr(x, y, items[idx]);
    _u8g2.setDrawColor(1);
  }

  _u8g2.drawStr(x, 63, "OK:select  BACK:exit");
}

void Ui::drawEditBuzzerEnabled(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "BUZZER ENABLED");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = m.buzzerEnabled ? "ON" : "OFF";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:toggle  OK:save  BACK:cancel");
}

void Ui::drawEditBuzzerStepFinished(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "STEP DONE BEEP");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = m.buzzerStepFinished ? "ON" : "OFF";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:toggle  OK:save  BACK:cancel");
}

void Ui::drawEditBuzzerProcessEnded(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "PROCESS END BEEP");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = m.buzzerProcessEnded ? "ON" : "OFF";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:toggle  OK:save  BACK:cancel");
}

void Ui::drawEditBuzzerTempWarning(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "TEMP WARNING BEEP");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = m.buzzerTempWarning ? "ON" : "OFF";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:toggle  OK:save  BACK:cancel");
}

void Ui::drawEditBuzzerFreq(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "BUZZER FREQUENCY");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", m.buzzerFreq);
  int w = _u8g2.getStrWidth(buf);
  _u8g2.drawStr((128 - w) / 2, 48, buf);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:+/-100  OK:save  BACK:cancel");
}

void Ui::drawBuzzerTest(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "BUZZER TEST");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_10x20_tf);
  _u8g2.drawStr(x, 40, "Press OK to beep");

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "OK:test  BACK:exit");
}

// ===== Hardware Menu =====
void Ui::drawHardwareMenu(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "HARDWARE");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_5x8_tf);
  const char* items[] = {
    "Steps/rev", "Microsteps", "Driver", "Invert dir", 
    "Buzzer type", "Buzz active", "Temp offset"
  };
  const int8_t itemCount = 7;
  
  int8_t startY = 26;
  int8_t lineH = 10;
  int8_t visible = 4;
  int8_t scroll = 0;
  if (m.subMenuIdx >= visible) scroll = m.subMenuIdx - visible + 1;
  
  for (int8_t i = 0; i < visible && (scroll + i) < itemCount; i++) {
    int8_t idx = scroll + i;
    int y = startY + i * lineH;
    if (idx == m.subMenuIdx) {
      _u8g2.drawBox(0, y - 8, 128, lineH);
      _u8g2.setDrawColor(0);
    }
    _u8g2.drawStr(x, y, items[idx]);
    _u8g2.setDrawColor(1);
  }

  _u8g2.drawStr(x, 63, "OK:select  BACK:exit");
}

void Ui::drawEditStepsPerRev(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "STEPS PER REV");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", m.stepsPerRev);
  int w = _u8g2.getStrWidth(buf);
  _u8g2.drawStr((128 - w) / 2, 48, buf);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:200/400  OK:save  BACK:cancel");
}

void Ui::drawEditMicrosteps(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "MICROSTEPS");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", m.microsteps);
  int w = _u8g2.getStrWidth(buf);
  _u8g2.drawStr((128 - w) / 2, 48, buf);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:cycle  OK:save  BACK:cancel");
}

void Ui::drawEditDriverType(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "DRIVER TYPE");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = (m.driverType == 0) ? "A4988" : "TMC";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:toggle  OK:save  BACK:cancel");
}

void Ui::drawEditMotorInvert(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "INVERT DIRECTION");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = m.motorInvert ? "YES" : "NO";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:toggle  OK:save  BACK:cancel");
}

void Ui::drawEditBuzzerType(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "BUZZER TYPE");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = (m.buzzerType == 0) ? "Active" : "Passive";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:toggle  OK:save  BACK:cancel");
}

void Ui::drawEditBuzzerActiveHigh(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "BUZZER ACTIVE");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = m.buzzerActiveHigh ? "HIGH" : "LOW";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:toggle  OK:save  BACK:cancel");
}

void Ui::drawEditTempOffset(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "TEMP CALIBRATION");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  char buf[16];
  snprintf(buf, sizeof(buf), "%+.1f", m.tempOffset);
  int w = _u8g2.getStrWidth(buf);
  _u8g2.drawStr((128 - w) / 2, 48, buf);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:+/-0.1  OK:save  BACK:cancel");
}
