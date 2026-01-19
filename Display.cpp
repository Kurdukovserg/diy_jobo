#include "Display.h"
#include "Config.h"

void Display::begin(uint8_t sda, uint8_t scl) {
  // Use software I2C for ESP8266 with custom pins
  _u8g2 = new U8G2_SSD1306_128X64_NONAME_F_SW_I2C(U8G2_R0, scl, sda, U8X8_PIN_NONE);
  _u8g2->begin();

  _initialized = true;
  _lastDrawMs = 0;
}

void Display::tick(const Status& st, float currentRpm, float targetRpm, float tempC, bool hasTemp, bool isMotorActive) {
  if (!_initialized) return;

  uint32_t now = millis();
  
  // Slower updates when motor active (but still update - stepping is interrupt-driven)
  uint32_t interval = isMotorActive ? 250 : UI_UPDATE_MS;
  if (now - _lastDrawMs < interval) return;
  _lastDrawMs = now;

  _u8g2->clearBuffer();

  const int x = 3;

  // State line
  _u8g2->setFont(u8g2_font_6x13B_tf);
  const char* stateStr = "IDLE";
  switch (st.state) {
    case ProcState::Running:  stateStr = "RUN"; break;
    case ProcState::Paused:   stateStr = "PAUSE"; break;
    case ProcState::Stopping: stateStr = "STOP..."; break;
    default: stateStr = "IDLE"; break;
  }
  _u8g2->drawStr(x, 12, stateStr);

  // Direction indicator
  const char* dirStr = st.dirFwd ? "FWD" : "REV";
  _u8g2->drawStr(70, 12, dirStr);

  // Reversing indicator
  if (st.reversing) {
    _u8g2->drawStr(100, 12, "<>");
  }

  // Target RPM
  _u8g2->setFont(u8g2_font_6x13_tf);
  char buf[24];
  snprintf(buf, sizeof(buf), "SET: %d RPM", (int)targetRpm);
  _u8g2->drawStr(x, 28, buf);

  // Current RPM
  snprintf(buf, sizeof(buf), "CUR: %.1f RPM", currentRpm);
  _u8g2->drawStr(x, 42, buf);

  // Temperature
  if (hasTemp && !isnan(tempC)) {
    snprintf(buf, sizeof(buf), "T: %.1f C", tempC);
  } else {
    snprintf(buf, sizeof(buf), "T: --.- C");
  }
  _u8g2->drawStr(x, 56, buf);

  _u8g2->sendBuffer();
}
