#pragma once
#include <Arduino.h>
#include <U8g2lib.h>

// Forward declare Screen enum
enum class Screen : uint8_t;

// Forward declare TempCoefTarget
enum class TempCoefTarget : uint8_t;

struct UiModel {
  Screen screen;
  int8_t menuIdx = 0;
  int8_t subMenuIdx = 0;
  
  bool run = false;
  int32_t rpm = 0;
  float currentRpm = 0.0f;
  bool dirFwd = true;
  
  // Auto-reverse
  bool reverseEnabled = false;
  float reverseIntervalSec = 0.0f;

  // Process steps
  int8_t stepCount = 1;
  int8_t currentStep = 0;
  int32_t stepRemainingSec = 0;
  int32_t totalRemainingSec = 0;
  bool isPaused = false;  // paused between steps
  
  // For edit screen
  int8_t editStepIdx = 0;
  int32_t editStepDuration = 0;

  // Temperature coefficient
  bool tempCoefEnabled = false;
  float tempCoefBase = 20.0f;
  float tempCoefPercent = 10.0f;
  TempCoefTarget tempCoefTarget;

  bool hasTemp = false;
  float tempC = NAN;

  // live states (debug)
  bool okDown = false;
  bool backDown = false;
  bool a0Down = false;
  bool encSwDown = false;

  // one-shot events
  bool okEvent = false;
  bool backEvent = false;
  bool a0BackEvent = false;
  bool encSwEvent = false;
  bool encSwRawHigh = true;
};



class Ui {
public:
  void begin();
  void tick(const UiModel& m);

private:
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C _u8g2{U8G2_R0, U8X8_PIN_NONE};
  uint32_t _lastDraw = 0;

  void draw(const UiModel& m);
  void drawMain(const UiModel& m);
  void drawMenu(const UiModel& m);
  void drawEditRpm(const UiModel& m);
  // Steps
  void drawStepsMenu(const UiModel& m);
  void drawEditStepDuration(const UiModel& m);
  // Reverse
  void drawReverseMenu(const UiModel& m);
  void drawEditReverseEnabled(const UiModel& m);
  void drawEditReverseInterval(const UiModel& m);
  // TempCoef
  void drawTempCoefMenu(const UiModel& m);
  void drawEditTempCoefEnabled(const UiModel& m);
  void drawEditTempCoefBase(const UiModel& m);
  void drawEditTempCoefPercent(const UiModel& m);
  void drawEditTempCoefTarget(const UiModel& m);
};
