#pragma once
#include <Arduino.h>

class StepperISR {
public:
  void begin(uint8_t stepPin, uint8_t dirPin);

  // signed steps/sec: + forward, - reverse
  void setSpeedSps(float sps);

  // immediate stop (interval=0)
  void stop();
  
  // Force timer to fire soon (call after stop->start transition)
  void kickStart();

  // ISR handler
#if defined(ESP8266)
  static void ICACHE_RAM_ATTR onTimer();
#else
  static void IRAM_ATTR onTimer();
#endif

private:
  static StepperISR* self;

  uint8_t _stepPin = 0;
  uint8_t _dirPin  = 0;

public:
  // Shared with ISR (keep it primitive)
  volatile uint32_t _intervalUs = 0; // 0 => stopped
  volatile bool     _dirFwd     = true;
  volatile bool     _enabled    = false;
  volatile uint32_t pulseCount = 0;  // Debug: count pulses sent
  volatile uint32_t isrCount = 0;    // Debug: count ISR fires

#if defined(ESP32)
  void* _timer = nullptr; // hw_timer_t*
#endif
};

extern StepperISR stepperISR;
