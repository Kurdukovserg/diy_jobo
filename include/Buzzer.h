#pragma once
#include <Arduino.h>

class Buzzer {
public:
  void begin(uint8_t pin);
  void beep(uint16_t freqHz = 1000, uint16_t durationMs = 100);
  void tick();  // call from loop to handle non-blocking beep
  
  bool enabled = true;
  
private:
  uint8_t _pin = 0;
  uint32_t _beepEndMs = 0;
  bool _beeping = false;
};

extern Buzzer buzzer;
