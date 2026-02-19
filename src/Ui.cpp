#include "Ui.h"
#include "Config.h"
#include "MenuController.h"
#include "SessionController.h"
#include "Buzzer.h"
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
    case Screen::EditRpm:   drawEditRpm(m); break;
    // Steps
    case Screen::StepsMenu: drawStepsMenu(m); break;
    case Screen::EditStepDuration: drawEditStepDuration(m); break;
    // Reverse
    case Screen::ReverseMenu: drawReverseMenu(m); break;
    case Screen::EditReverseEnabled: drawEditReverseEnabled(m); break;
    case Screen::EditReverseInterval: drawEditReverseInterval(m); break;
    // TempCoef
    case Screen::TempCoefMenu: drawTempCoefMenu(m); break;
    case Screen::EditTempCoefEnabled: drawEditTempCoefEnabled(m); break;
    case Screen::EditTempCoefBase: drawEditTempCoefBase(m); break;
    case Screen::EditTempCoefPercent: drawEditTempCoefPercent(m); break;
    case Screen::EditTempCoefTarget: drawEditTempCoefTarget(m); break;
    // Temp limits
    case Screen::EditTempLimitsEnabled: drawEditTempLimitsEnabled(m); break;
    case Screen::EditTempMin: drawEditTempMin(m); break;
    case Screen::EditTempMax: drawEditTempMax(m); break;
    // Buzzer
    case Screen::BuzzerMenu: drawBuzzerMenu(m); break;
    case Screen::EditBuzzerEnabled: drawEditBuzzerEnabled(m); break;
  }

  _u8g2.sendBuffer();
}

void Ui::drawMain(const UiModel& m) {
  const int x = 2;

  // Header
  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 9, "DIY JOBO");
  
  char statusBuf[16];
  if (m.run) {
    snprintf(statusBuf, sizeof(statusBuf), "%s%s", 
             m.dirFwd ? "FWD" : "REV",
             m.reverseEnabled ? "~" : "");
  } else {
    snprintf(statusBuf, sizeof(statusBuf), "STOP");
  }
  _u8g2.drawStr(80, 9, statusBuf);
  _u8g2.drawHLine(x, 11, 124);

  // RPM - show adjusted value if temp coef is active and affects RPM
  _u8g2.setFont(u8g2_font_6x13_tf);
  char buf[32];
  bool rpmAdjusted = m.tempCoefEnabled && 
    (m.tempCoefTarget == TempCoefTarget::Rpm || m.tempCoefTarget == TempCoefTarget::Both) &&
    m.hasTemp && !isnan(m.tempC) && (int32_t)m.adjustedRpm != m.rpm;
  if (rpmAdjusted) {
    snprintf(buf, sizeof(buf), "RPM: %ld>%.0f (%.0f)", (long)m.rpm, m.adjustedRpm, m.currentRpm);
  } else {
    snprintf(buf, sizeof(buf), "RPM: %ld (%.0f)", (long)m.rpm, m.currentRpm);
  }
  _u8g2.drawStr(x, 26, buf);

  // Temperature + coefficient indicator + alarm
  char tbuf[32];
  if (!m.hasTemp || isnan(m.tempC)) {
    snprintf(tbuf, sizeof(tbuf), "T: --.-C%s", m.tempCoefEnabled ? " *" : "");
  } else {
    const char* alarmStr = "";
    if (m.tempAlarm) alarmStr = m.tempLow ? " LOW!" : " HIGH!";
    snprintf(tbuf, sizeof(tbuf), "T: %.1fC%s%s", m.tempC, m.tempCoefEnabled ? "*" : "", alarmStr);
  }
  _u8g2.drawStr(x, 40, tbuf);

  // Step progress
  _u8g2.setFont(u8g2_font_5x8_tf);
  char stepBuf[32];
  if (m.stepCount > 0 && m.totalRemainingSec > 0) {
    int mins = m.stepRemainingSec / 60;
    int secs = m.stepRemainingSec % 60;
    if (m.isPaused) {
      snprintf(stepBuf, sizeof(stepBuf), "STEP %d/%d DONE - OK:next", m.currentStep + 1, m.stepCount);
    } else {
      snprintf(stepBuf, sizeof(stepBuf), "Step %d/%d: %d:%02d", m.currentStep + 1, m.stepCount, mins, secs);
    }
  } else {
    snprintf(stepBuf, sizeof(stepBuf), "Steps: %d", m.stepCount);
  }
  _u8g2.drawStr(x, 52, stepBuf);

  // Hint line
  if (m.isPaused) {
    _u8g2.drawStr(x, 63, "OK:next step  BACK:stop");
  } else {
    _u8g2.drawStr(x, 63, "OK:run ENC:menu BACK:stop");
  }
}

void Ui::drawMenu(const UiModel& m) {
  const int x = 2;
  const int TOTAL = 5;  // RPM, Steps>, Reverse>, TempCoef>, Buzzer>
  const int VISIBLE = 4;
  
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "SETTINGS");
  _u8g2.drawHLine(x, 14, 124);

  const char* items[] = {"RPM", "Steps", "Reverse", "TempCoef", "Buzzer"};
  char buf[32];
  
  int scrollOff = 0;
  if (m.menuIdx >= VISIBLE) scrollOff = m.menuIdx - VISIBLE + 1;
  
  for (int v = 0; v < VISIBLE && (v + scrollOff) < TOTAL; v++) {
    int i = v + scrollOff;
    int y = 26 + v * 10;
    bool sel = (i == m.menuIdx);
    
    if (sel) {
      _u8g2.drawBox(0, y - 8, 128, 10);
      _u8g2.setDrawColor(0);
    }
    
    switch (i) {
      case 0: snprintf(buf, sizeof(buf), "%s: %ld", items[i], (long)m.rpm); break;
      case 1: snprintf(buf, sizeof(buf), "%s (%d) >", items[i], m.stepCount); break;
      case 2: snprintf(buf, sizeof(buf), "%s >", items[i]); break;
      case 3: snprintf(buf, sizeof(buf), "%s >", items[i]); break;
      case 4: snprintf(buf, sizeof(buf), "%s: %s >", items[i], buzzer.enabled ? "ON" : "OFF"); break;
    }
    _u8g2.drawStr(x, y, buf);
    _u8g2.setDrawColor(1);
  }
  
  // Scroll indicators
  _u8g2.setFont(u8g2_font_5x8_tf);
  if (scrollOff > 0) _u8g2.drawStr(120, 20, "^");
  if (scrollOff + VISIBLE < TOTAL) _u8g2.drawStr(120, 54, "v");

  _u8g2.drawStr(x, 63, "ENC:sel  OK:edit  BACK:exit");
}

void Ui::drawEditRpm(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "SET RPM");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  char buf[8];
  snprintf(buf, sizeof(buf), "%ld", (long)m.rpm);
  int w = _u8g2.getStrWidth(buf);
  _u8g2.drawStr((128 - w) / 2, 48, buf);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:adj  OK:save  BACK:cancel");
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

// ===== Buzzer Submenu =====
void Ui::drawBuzzerMenu(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "BUZZER");
  _u8g2.drawHLine(x, 14, 124);

  char buf[32];
  
  for (int i = 0; i < 2; i++) {
    int y = 30 + i * 14;
    bool sel = (i == m.subMenuIdx);
    if (sel) {
      _u8g2.drawBox(0, y - 10, 128, 13);
      _u8g2.setDrawColor(0);
    }
    if (i == 0) {
      snprintf(buf, sizeof(buf), "Enabled: %s", buzzer.enabled ? "ON" : "OFF");
    } else {
      snprintf(buf, sizeof(buf), "Test Beep");
    }
    _u8g2.drawStr(x, y, buf);
    _u8g2.setDrawColor(1);
  }
  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "OK:edit/test  BACK:menu");
}

void Ui::drawEditBuzzerEnabled(const UiModel& m) {
  const int x = 2;
  _u8g2.setFont(u8g2_font_6x13_tf);
  _u8g2.drawStr(x, 12, "BUZZER ENABLED");
  _u8g2.drawHLine(x, 14, 124);

  _u8g2.setFont(u8g2_font_fur30_tf);
  const char* val = buzzer.enabled ? "ON" : "OFF";
  int w = _u8g2.getStrWidth(val);
  _u8g2.drawStr((128 - w) / 2, 48, val);

  _u8g2.setFont(u8g2_font_5x8_tf);
  _u8g2.drawStr(x, 63, "ENC:toggle  OK:save  BACK:cancel");
}
