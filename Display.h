#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include "Types.h"

class Display {
public:
  void begin(uint8_t sda, uint8_t scl);
  // isMotorActive: if true, skip update to avoid blocking stepper
  void tick(const Status& st, float currentRpm, float targetRpm, float tempC, bool hasTemp, bool isMotorActive);

private:
  U8G2_SSD1306_128X64_NONAME_F_SW_I2C* _u8g2 = nullptr;
  uint32_t _lastDrawMs = 0;
  bool _initialized = false;
};
