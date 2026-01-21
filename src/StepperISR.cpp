#include "StepperISR.h"
#include <math.h>

StepperISR* StepperISR::self = nullptr;
StepperISR stepperISR;

static inline uint32_t spsToIntervalUs(float spsAbs) {
  if (spsAbs < 0.5f) return 0;

  float us = 1000000.0f / spsAbs;
  // clamp (avoid silly small intervals)
  if (us < 40.0f) us = 40.0f;
  return (uint32_t)us;
}

void StepperISR::begin(uint8_t stepPin, uint8_t dirPin) {
  self = this;
  _stepPin = stepPin;
  _dirPin  = dirPin;

  pinMode(_stepPin, OUTPUT);
  pinMode(_dirPin, OUTPUT);
  digitalWrite(_stepPin, LOW);
  digitalWrite(_dirPin, LOW);

  _enabled = true;

#if defined(ESP32)
  // 1 tick = 1us (80MHz / 80)
  hw_timer_t* t = timerBegin(0, 80, true);
  _timer = (void*)t;
  timerAttachInterrupt(t, &StepperISR::onTimer, true);
  timerAlarmWrite(t, 20000, true); // slow idle tick
  timerAlarmEnable(t);

#elif defined(ESP8266)
  // timer1: we will re-arm it each ISR (single shot)
  timer1_isr_init();
  timer1_attachInterrupt(&StepperISR::onTimer);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(20000 * 5); // ~20ms idle tick (ticks ≈ us*5 at DIV16)
#endif
}

void StepperISR::stop() {
  noInterrupts();
  _intervalUs = 0;
  interrupts();
}

void StepperISR::setSpeedSps(float sps) {
  bool dir = (sps >= 0.0f);
  float spsAbs = fabsf(sps);

  uint32_t intervalUs = spsToIntervalUs(spsAbs);

  // atomic-ish update
  noInterrupts();
  _dirFwd = dir;
  _intervalUs = intervalUs;
  interrupts();
}

#if defined(ESP8266)
void ICACHE_RAM_ATTR StepperISR::onTimer() {
#else
void IRAM_ATTR StepperISR::onTimer() {
#endif
  StepperISR* s = self;
  if (!s || !s->_enabled) return;

  uint32_t intervalUs = s->_intervalUs;

  if (intervalUs == 0) {
    // stopped => keep slow tick
#if defined(ESP32)
    hw_timer_t* t = (hw_timer_t*)s->_timer;
    timerAlarmWrite(t, 20000, true);
#else
    timer1_write(20000 * 5);
#endif
    return;
  }

  digitalWrite(s->_dirPin, s->_dirFwd ? HIGH : LOW);

  // Pulse: HIGH then LOW immediately.
  // digitalWrite latency usually gives enough pulse width for TMC2209.
  digitalWrite(s->_stepPin, HIGH);
  digitalWrite(s->_stepPin, LOW);

#if defined(ESP32)
  hw_timer_t* t = (hw_timer_t*)s->_timer;
  timerAlarmWrite(t, intervalUs, true);
#else
  timer1_write(intervalUs * 5);
#endif
}
