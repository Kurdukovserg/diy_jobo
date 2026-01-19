#pragma once
#include <Arduino.h>

enum class ProcState : uint8_t { Idle, Running, Paused, Stopping };
enum class RevPhase  : uint8_t { None, RampDown, Pause, RampUp };
enum class RampPhase : uint8_t { None, Up, Down };
enum class AutoRevMode : uint8_t { Off, Time, Turns };
enum class WifiMode : uint8_t { Ap, Sta, ApSta };

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

  int rpm = 10;
  bool dirFwd = true;
};

struct Status {
  ProcState state = ProcState::Idle;
  int rpm = 10;
  bool dirFwd = true;
  bool reversing = false;

  RevParams rev;
  MotionParams motion;
  AutoRevConfig autoRev;

  float tempC = 37.8f; // mock
};
