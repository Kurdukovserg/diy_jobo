#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include "Types.h"

class Display {
public:
  void begin(uint8_t sda, uint8_t scl);
  void tick(const Status& st, const PersistentConfig& cfg, float currentRpm, float tempC, bool hasTemp, 
            bool isMotorActive, UIScreen screen, int menuIndex, bool editMode);

private:
  void drawMainScreen(const Status& st, const TempConfig& tc, float currentRpm, float targetRpm, float tempC, bool hasTemp);
  void drawMenuMain(int idx);
  void drawMenuAutoRev(const AutoRevConfig& ar, int idx, bool editing);
  void drawMenuAccel(const MotionParams& mp, int idx, bool editing);
  void drawMenuTemp(const TempConfig& tc, int idx, bool editing);
  void drawMenuProcess(uint16_t timeSec, int idx, bool editing);
  void drawMenuWiFi(const WifiConfig& wifi);
  void drawStopConfirm(int idx);

  U8G2_SSD1306_128X64_NONAME_F_SW_I2C* _u8g2 = nullptr;
  uint32_t _lastDrawMs = 0;
  bool _initialized = false;
};
