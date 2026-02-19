#include "Buzzer.h"

Buzzer buzzer;

void Buzzer::begin(uint8_t pin) {
  _pin = pin;
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}

void Buzzer::beep(uint16_t freqHz, uint16_t durationMs) {
  if (!enabled) return;
  
  // Manual GPIO toggle - avoids Timer1 conflict with StepperISR
  uint32_t periodUs = 1000000UL / freqHz;
  uint32_t halfPeriodUs = periodUs / 2;
  uint32_t cycles = (uint32_t)freqHz * durationMs / 1000;
  
  for (uint32_t i = 0; i < cycles; i++) {
    digitalWrite(_pin, HIGH);
    delayMicroseconds(halfPeriodUs);
    digitalWrite(_pin, LOW);
    delayMicroseconds(halfPeriodUs);
  }
}

void Buzzer::tick() {
  // No longer needed for blocking beep, but keep for future non-blocking
}
