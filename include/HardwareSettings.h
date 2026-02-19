#pragma once
#include <Arduino.h>

enum class BuzzerType : uint8_t {
  Active = 0,   // Simple on/off buzzer
  Passive = 1   // Needs tone() for sound
};

enum class DriverType : uint8_t {
  Generic = 0,  // A4988, DRV8825, etc.
  TMC2209 = 1   // TMC2209 (StealthChop)
};

struct HardwareSettings {
  // Motor
  uint16_t stepsPerRev = 200;      // 200 for 1.8°, 400 for 0.9°
  uint8_t microsteps = 16;         // 1, 2, 4, 8, 16, 32
  DriverType driverType = DriverType::TMC2209;
  bool motorInvertDir = false;     // Invert motor direction
  
  // Buzzer
  BuzzerType buzzerType = BuzzerType::Passive;
  bool buzzerActiveHigh = true;    // Active level for buzzer
  
  // Temperature
  float tempOffset = 0.0f;         // Calibration offset (-5.0 to +5.0)
  
  // Computed
  uint32_t stepsEffective() const {
    return (uint32_t)stepsPerRev * microsteps;
  }
};
