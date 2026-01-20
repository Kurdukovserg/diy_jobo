#pragma once
#include <Arduino.h>

enum class ProcState : uint8_t { Idle, Running, Paused, Stopping };
enum class RevPhase  : uint8_t { None, RampDown, Pause, RampUp };
enum class RampPhase : uint8_t { None, Up, Down };
enum class AutoRevMode : uint8_t { Off, Time, Turns };
enum class WifiMode : uint8_t { Ap, Sta, ApSta };
enum class UIScreen : uint8_t { 
  Main,           // Main status display
  Menu,           // Main menu
  AutoRev,        // Auto-reverse sub-menu
  AutoRevEdit,    // Editing auto-reverse value
  Accel,          // Acceleration sub-menu
  AccelEdit,      // Editing acceleration value
  TempMenu,       // Temperature settings sub-menu
  TempEdit,       // Editing temperature value
  ProcessMenu,    // Process time sub-menu
  ProcessEdit,    // Editing process time
  WiFiMenu,       // WiFi info sub-menu
  StopConfirm,    // Stop confirmation dialog
};

struct RevParams {
  uint16_t rampMs = 350;
  uint16_t pauseMs = 150;
};

struct MotionParams {
  uint16_t startRampMs = 350;
  uint16_t stopRampMs  = 600;
};

struct AutoRevConfig {
  AutoRevMode mode = AutoRevMode::Off;
  uint16_t interval = 0; // seconds (time) or turns (turns)
};

struct TempConfig {
  float baseTemp = 20.0f;    // Base/reference temperature (°C)
  float coefficient = 1.0f;  // Time adjustment per °C deviation (0.5-2.0)
  float alarmThreshold = 3.0f; // Alarm if deviation > this (°C)
  bool enabled = false;      // Enable temperature compensation
};

struct WifiConfig {
  WifiMode mode = WifiMode::ApSta;  // WLED-like default
  char staSsid[33] = "";
  char staPass[65] = "";
  uint8_t apSubnet = 4;             // 192.168.4.1 in AP fallback
};

struct PersistentConfig {
  uint16_t ver = 1;
  WifiConfig wifi;

  RevParams rev;
  MotionParams motion;
  AutoRevConfig autoRev;
  TempConfig temp;

  int rpm = 10;
  bool dirFwd = true;
  uint16_t processTimeSec = 300;  // Default 5 minutes
};

struct Status {
  ProcState state = ProcState::Idle;
  int rpm = 10;
  bool dirFwd = true;
  bool reversing = false;

  RevParams rev;
  MotionParams motion;
  AutoRevConfig autoRev;

  float tempC = 37.8f;
  
  // Timer
  uint32_t timerStartMs = 0;      // When timer started
  uint16_t timerDurationSec = 0;  // Adjusted duration (with temp factor)
  uint16_t timerPausedSec = 0;    // Elapsed seconds when paused
  bool timerActive = false;
  
  // Temperature alarm
  bool tempAlarm = false;         // True when temp deviates beyond threshold
};
