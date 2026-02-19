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
  float adjustedRpm = 0.0f;  // RPM after temp coefficient applied
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
  int32_t stepDurations[10] = {0};  // durations for display in StepsMenu
  
  // For edit screen
  int8_t editStepIdx = 0;
  int32_t editStepDuration = 0;

  // Temperature coefficient
  bool tempCoefEnabled = false;
  float tempCoefBase = 20.0f;
  float tempCoefPercent = 10.0f;
  TempCoefTarget tempCoefTarget;
  
  // Temperature limits
  bool tempLimitsEnabled = false;
  float tempMin = 18.0f;
  float tempMax = 24.0f;
  bool tempAlarm = false;
  bool tempLow = false;
  bool tempHigh = false;

  // Buzzer settings
  bool buzzerEnabled = true;
  bool buzzerStepFinished = true;
  bool buzzerProcessEnded = true;
  bool buzzerTempWarning = true;
  uint16_t buzzerFreq = 2200;

  // Hardware settings
  uint16_t stepsPerRev = 200;
  uint8_t microsteps = 16;
  uint8_t driverType = 1;      // 0=Generic, 1=TMC2209
  bool motorInvert = false;
  uint8_t buzzerType = 1;      // 0=Active, 1=Passive
  bool buzzerActiveHigh = true;
  float tempOffset = 0.0f;

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
  // Temp limits
  void drawEditTempLimitsEnabled(const UiModel& m);
  void drawEditTempMin(const UiModel& m);
  void drawEditTempMax(const UiModel& m);
  // Buzzer
  void drawBuzzerMenu(const UiModel& m);
  void drawEditBuzzerEnabled(const UiModel& m);
  void drawEditBuzzerStepFinished(const UiModel& m);
  void drawEditBuzzerProcessEnded(const UiModel& m);
  void drawEditBuzzerTempWarning(const UiModel& m);
  void drawEditBuzzerFreq(const UiModel& m);
  void drawBuzzerTest(const UiModel& m);
  // Hardware
  void drawHardwareMenu(const UiModel& m);
  void drawEditStepsPerRev(const UiModel& m);
  void drawEditMicrosteps(const UiModel& m);
  void drawEditDriverType(const UiModel& m);
  void drawEditMotorInvert(const UiModel& m);
  void drawEditBuzzerType(const UiModel& m);
  void drawEditBuzzerActiveHigh(const UiModel& m);
  void drawEditTempOffset(const UiModel& m);
};
