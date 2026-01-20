#include "Display.h"
#include "Config.h"

void Display::begin(uint8_t sda, uint8_t scl) {
  _u8g2 = new U8G2_SSD1306_128X64_NONAME_F_SW_I2C(U8G2_R0, scl, sda, U8X8_PIN_NONE);
  _u8g2->begin();
  _initialized = true;
  _lastDrawMs = 0;
}

void Display::tick(const Status& st, const PersistentConfig& cfg, float currentRpm, float tempC, bool hasTemp, 
                   bool isMotorActive, UIScreen screen, int menuIndex, bool editMode) {
  if (!_initialized) return;

  uint32_t now = millis();
  uint32_t interval = isMotorActive ? 250 : UI_UPDATE_MS;
  if (now - _lastDrawMs < interval) return;
  _lastDrawMs = now;

  _u8g2->clearBuffer();

  switch (screen) {
    case UIScreen::Main:
      drawMainScreen(st, cfg.temp, currentRpm, (float)cfg.rpm, tempC, hasTemp);
      break;
    case UIScreen::Menu:
      drawMenuMain(menuIndex);
      break;
    case UIScreen::AutoRev:
    case UIScreen::AutoRevEdit:
      drawMenuAutoRev(cfg.autoRev, menuIndex, screen == UIScreen::AutoRevEdit);
      break;
    case UIScreen::Accel:
    case UIScreen::AccelEdit:
      drawMenuAccel(cfg.motion, menuIndex, screen == UIScreen::AccelEdit);
      break;
    case UIScreen::TempMenu:
    case UIScreen::TempEdit:
      drawMenuTemp(cfg.temp, menuIndex, screen == UIScreen::TempEdit);
      break;
    case UIScreen::ProcessMenu:
    case UIScreen::ProcessEdit:
      drawMenuProcess(cfg.processTimeSec, menuIndex, screen == UIScreen::ProcessEdit);
      break;
    case UIScreen::WiFiMenu:
      drawMenuWiFi(cfg.wifi);
      break;
    case UIScreen::StopConfirm:
      drawStopConfirm(menuIndex);
      break;
  }

  _u8g2->sendBuffer();
}

void Display::drawMainScreen(const Status& st, const TempConfig& tc, float currentRpm, float targetRpm, float tempC, bool hasTemp) {
  const int x = 3;
  char buf[24];

  _u8g2->setFont(u8g2_font_6x13B_tf);
  
  // Line 1: State + Timer countdown
  if (st.timerActive && st.state == ProcState::Running) {
    uint32_t elapsedSec = (millis() - st.timerStartMs) / 1000;
    int32_t remainSec = (int32_t)st.timerDurationSec - (int32_t)elapsedSec;
    if (remainSec < 0) remainSec = 0;
    int mins = remainSec / 60;
    int secs = remainSec % 60;
    snprintf(buf, sizeof(buf), "RUN %02d:%02d", mins, secs);
    _u8g2->drawStr(x, 12, buf);
  } else {
    const char* stateStr = "IDLE";
    switch (st.state) {
      case ProcState::Running:  stateStr = "RUN"; break;
      case ProcState::Paused:   stateStr = "PAUSE"; break;
      case ProcState::Stopping: stateStr = "STOP..."; break;
      default: break;
    }
    _u8g2->drawStr(x, 12, stateStr);
  }

  const char* dirStr = st.dirFwd ? "FWD" : "REV";
  _u8g2->drawStr(85, 12, dirStr);

  if (st.reversing) {
    _u8g2->drawStr(110, 12, "<>");
  }

  _u8g2->setFont(u8g2_font_6x13_tf);
  
  // Line 2: RPM
  snprintf(buf, sizeof(buf), "RPM: %d / %.1f", (int)targetRpm, currentRpm);
  _u8g2->drawStr(x, 28, buf);

  // Line 3: Temperature with factor and alarm indicator
  if (hasTemp && !isnan(tempC)) {
    if (tc.enabled) {
      float delta = tempC - tc.baseTemp;
      float factor = 1.0f - delta * tc.coefficient * 0.01f;
      if (st.tempAlarm) {
        // Blinking alarm: show "!!" when alarm active
        snprintf(buf, sizeof(buf), "T:%.1fC !! %.2f", tempC, factor);
      } else {
        snprintf(buf, sizeof(buf), "T:%.1fC *%.2f", tempC, factor);
      }
    } else {
      snprintf(buf, sizeof(buf), "T: %.1f C", tempC);
    }
  } else {
    snprintf(buf, sizeof(buf), "T: --.- C");
  }
  _u8g2->drawStr(x, 42, buf);

  // Line 4: Hints based on state
  if (st.state == ProcState::Idle && st.timerDurationSec > 0) {
    int mins = st.timerDurationSec / 60;
    int secs = st.timerDurationSec % 60;
    snprintf(buf, sizeof(buf), "Time: %02d:%02d", mins, secs);
    _u8g2->drawStr(x, 56, buf);
  } else if (st.state == ProcState::Idle) {
    _u8g2->drawStr(x, 56, "Enc:start TX:menu");
  } else if (st.state == ProcState::Running) {
    _u8g2->drawStr(x, 56, "Enc:|| TX:menu Bk:X");
  } else if (st.state == ProcState::Paused) {
    _u8g2->drawStr(x, 56, "Enc:> TX:menu Bk:X");
  } else {
    _u8g2->drawStr(x, 56, "Stopping...");
  }
}

void Display::drawMenuMain(int idx) {
  const int x = 3;
  _u8g2->setFont(u8g2_font_6x13B_tf);
  _u8g2->drawStr(x, 12, "MENU");
  
  _u8g2->setFont(u8g2_font_6x13_tf);
  const char* items[] = {"Auto-Reverse", "Acceleration", "Temperature", "Process", "WiFi"};
  const int itemCount = 5;
  
  // Show up to 3 items with scrolling
  int startIdx = (idx > 1) ? idx - 1 : 0;
  if (startIdx > itemCount - 3) startIdx = itemCount - 3;
  
  for (int i = 0; i < 3 && (startIdx + i) < itemCount; i++) {
    int itemIdx = startIdx + i;
    int y = 28 + i * 14;
    if (itemIdx == idx) {
      _u8g2->drawStr(x, y, ">");
    }
    _u8g2->drawStr(x + 12, y, items[itemIdx]);
  }
}

void Display::drawMenuAutoRev(const AutoRevConfig& ar, int idx, bool editing) {
  const int x = 3;
  char buf[24];
  
  _u8g2->setFont(u8g2_font_6x13B_tf);
  _u8g2->drawStr(x, 12, "AUTO-REVERSE");
  
  _u8g2->setFont(u8g2_font_6x13_tf);
  
  // Mode line
  const char* modeStr = "Off";
  if (ar.mode == AutoRevMode::Time) modeStr = "Time";
  else if (ar.mode == AutoRevMode::Turns) modeStr = "Turns";
  
  snprintf(buf, sizeof(buf), "Mode: %s", modeStr);
  _u8g2->drawStr(x + 12, 28, buf);
  if (idx == 0) _u8g2->drawStr(x, 28, editing ? "*" : ">");
  
  // Interval line
  const char* unit = (ar.mode == AutoRevMode::Turns) ? "rev" : "sec";
  snprintf(buf, sizeof(buf), "Interval: %d %s", ar.interval, unit);
  _u8g2->drawStr(x + 12, 42, buf);
  if (idx == 1) _u8g2->drawStr(x, 42, editing ? "*" : ">");
  
  // Back
  _u8g2->drawStr(x + 12, 56, "< Back");
  if (idx == 2) _u8g2->drawStr(x, 56, ">");
}

void Display::drawMenuAccel(const MotionParams& mp, int idx, bool editing) {
  const int x = 3;
  char buf[24];
  
  _u8g2->setFont(u8g2_font_6x13B_tf);
  _u8g2->drawStr(x, 12, "ACCELERATION");
  
  _u8g2->setFont(u8g2_font_6x13_tf);
  
  snprintf(buf, sizeof(buf), "Start: %d ms", mp.startRampMs);
  _u8g2->drawStr(x + 12, 28, buf);
  if (idx == 0) _u8g2->drawStr(x, 28, editing ? "*" : ">");
  
  snprintf(buf, sizeof(buf), "Stop: %d ms", mp.stopRampMs);
  _u8g2->drawStr(x + 12, 42, buf);
  if (idx == 1) _u8g2->drawStr(x, 42, editing ? "*" : ">");
  
  _u8g2->drawStr(x + 12, 56, "< Back");
  if (idx == 2) _u8g2->drawStr(x, 56, ">");
}

void Display::drawMenuTemp(const TempConfig& tc, int idx, bool editing) {
  const int x = 3;
  char buf[24];
  
  _u8g2->setFont(u8g2_font_6x13B_tf);
  _u8g2->drawStr(x, 12, "TEMPERATURE");
  
  _u8g2->setFont(u8g2_font_6x13_tf);
  
  // 4 items: Enable, Base, Coeff, Alarm - show 3 with scrolling
  const char* labels[] = {"Enable:", "Base:", "Coeff:", "Alarm:"};
  int startIdx = (idx > 1) ? idx - 1 : 0;
  if (startIdx > 1) startIdx = 1;  // max scroll
  
  for (int i = 0; i < 3; i++) {
    int itemIdx = startIdx + i;
    if (itemIdx >= 4) break;
    int y = 28 + i * 14;
    
    switch (itemIdx) {
      case 0: snprintf(buf, sizeof(buf), "%s %s", labels[0], tc.enabled ? "ON" : "OFF"); break;
      case 1: snprintf(buf, sizeof(buf), "%s %.1f C", labels[1], tc.baseTemp); break;
      case 2: snprintf(buf, sizeof(buf), "%s %.2f", labels[2], tc.coefficient); break;
      case 3: snprintf(buf, sizeof(buf), "%s %.1f C", labels[3], tc.alarmThreshold); break;
    }
    _u8g2->drawStr(x + 12, y, buf);
    if (itemIdx == idx) _u8g2->drawStr(x, y, editing ? "*" : ">");
  }
}

void Display::drawMenuProcess(uint16_t timeSec, int idx, bool editing) {
  const int x = 3;
  char buf[24];
  
  _u8g2->setFont(u8g2_font_6x13B_tf);
  _u8g2->drawStr(x, 12, "PROCESS TIME");
  
  _u8g2->setFont(u8g2_font_6x13_tf);
  
  int mins = timeSec / 60;
  int secs = timeSec % 60;
  snprintf(buf, sizeof(buf), "Time: %02d:%02d", mins, secs);
  _u8g2->drawStr(x + 12, 28, buf);
  if (idx == 0) _u8g2->drawStr(x, 28, editing ? "*" : ">");
  
  _u8g2->drawStr(x + 12, 42, "< Back");
  if (idx == 1) _u8g2->drawStr(x, 42, ">");
  
  _u8g2->drawStr(x, 56, "Rotate to adjust");
}

void Display::drawMenuWiFi(const WifiConfig& wifi) {
  const int x = 3;
  char buf[32];
  
  _u8g2->setFont(u8g2_font_6x13B_tf);
  _u8g2->drawStr(x, 12, "WIFI INFO");
  
  _u8g2->setFont(u8g2_font_6x13_tf);
  
  const char* modeStr = "AP+STA";
  if (wifi.mode == WifiMode::Ap) modeStr = "AP";
  else if (wifi.mode == WifiMode::Sta) modeStr = "STA";
  snprintf(buf, sizeof(buf), "Mode: %s", modeStr);
  _u8g2->drawStr(x, 28, buf);
  
  if (wifi.staSsid[0]) {
    snprintf(buf, sizeof(buf), "SSID: %.12s", wifi.staSsid);
  } else {
    snprintf(buf, sizeof(buf), "SSID: (none)");
  }
  _u8g2->drawStr(x, 42, buf);
  
  _u8g2->drawStr(x, 56, "< Back (confirm)");
}

void Display::drawStopConfirm(int /*idx*/) {
  const int x = 3;
  
  _u8g2->setFont(u8g2_font_6x13B_tf);
  _u8g2->drawStr(x, 16, "STOP PROCESS?");
  
  _u8g2->setFont(u8g2_font_6x13_tf);
  _u8g2->drawStr(x, 36, "[BACK]  = Cancel");
  _u8g2->drawStr(x, 52, "[OK]    = Stop");
}
