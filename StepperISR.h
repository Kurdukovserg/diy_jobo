#pragma once
#include <Arduino.h>

class StepperISR {
public:
  void begin(uint8_t stepPin, uint8_t dirPin);
  
  // Set speed in steps per second (negative = reverse)
  void setSpeed(float stepsPerSec);
  
  // Get current speed
  float getSpeed() const { return _targetSps; }
  
  // Called by ISR - do not call directly
  static void IRAM_ATTR onTimer();

private:
  static StepperISR* _instance;
  
  uint8_t _stepPin = 0;
  uint8_t _dirPin = 0;
  
  volatile float _targetSps = 0;
  volatile bool _dirFwd = true;
  volatile uint32_t _stepInterval = 0;  // microseconds between steps
  volatile bool _enabled = false;
};

extern StepperISR stepperISR;
