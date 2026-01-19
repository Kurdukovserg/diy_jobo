#include "StepperISR.h"

StepperISR* StepperISR::_instance = nullptr;
StepperISR stepperISR;

#if defined(ESP32)
// ESP32: Use hw_timer
static hw_timer_t* stepTimer = nullptr;
static portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR StepperISR::onTimer() {
  if (!_instance || !_instance->_enabled) return;
  portENTER_CRITICAL_ISR(&timerMux);
  // Pulse STEP pin (rising edge triggers step on most drivers)
  GPIO.out_w1ts = (1 << _instance->_stepPin);  // HIGH
  delayMicroseconds(2);
  GPIO.out_w1tc = (1 << _instance->_stepPin);  // LOW
  portEXIT_CRITICAL_ISR(&timerMux);
}

#else
// ESP8266: Use timer1
extern "C" {
  #include "user_interface.h"
}

static volatile bool stepHigh = false;

void IRAM_ATTR StepperISR::onTimer() {
  if (!_instance || !_instance->_enabled) return;
  
  // Toggle step pin using direct register access for speed
  if (stepHigh) {
    GPOS = (1 << _instance->_stepPin);  // HIGH
  } else {
    GPOC = (1 << _instance->_stepPin);  // LOW
  }
  stepHigh = !stepHigh;
}

static void IRAM_ATTR timer1_isr_handler() {
  StepperISR::onTimer();
}
#endif

void StepperISR::begin(uint8_t stepPin, uint8_t dirPin) {
  _stepPin = stepPin;
  _dirPin = dirPin;
  _instance = this;
  
  pinMode(_stepPin, OUTPUT);
  pinMode(_dirPin, OUTPUT);
  digitalWrite(_stepPin, LOW);
  digitalWrite(_dirPin, LOW);
  
  _enabled = false;
  _targetSps = 0;

#if defined(ESP32)
  // ESP32: Setup timer (timer 0, prescaler 80 = 1MHz tick)
  stepTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(stepTimer, &onTimer, true);
  timerAlarmWrite(stepTimer, 1000, true);  // default 1ms
  timerAlarmDisable(stepTimer);
#else
  // ESP8266: Setup timer1
  timer1_isr_init();
  timer1_attachInterrupt(timer1_isr_handler);
  timer1_disable();
#endif
}

void StepperISR::setSpeed(float stepsPerSec) {
  _targetSps = stepsPerSec;
  
  float absSps = fabsf(stepsPerSec);
  
  if (absSps < 1.0f) {
    // Stop
    _enabled = false;
#if defined(ESP32)
    timerAlarmDisable(stepTimer);
#else
    timer1_disable();
#endif
    digitalWrite(_stepPin, LOW);
    return;
  }
  
  // Set direction (HIGH = forward for most drivers)
  bool newDir = (stepsPerSec >= 0);
  if (newDir != _dirFwd) {
    _dirFwd = newDir;
    digitalWrite(_dirPin, _dirFwd ? HIGH : LOW);
  }
  
#if defined(ESP32)
  // ESP32: One timer call per step (pulse in ISR)
  uint32_t intervalUs = (uint32_t)(1000000.0f / absSps);
  if (intervalUs < 50) intervalUs = 50;
  
  timerAlarmWrite(stepTimer, intervalUs, true);
  _enabled = true;
  timerAlarmEnable(stepTimer);
#else
  // ESP8266: Two timer calls per step (toggle)
  uint32_t intervalUs = (uint32_t)(1000000.0f / (absSps * 2.0f));
  if (intervalUs < 25) intervalUs = 25;
  
  // Timer1 uses 5MHz clock (DIV16 from 80MHz)
  // ticks = intervalUs * 5
  uint32_t ticks = intervalUs * 5;
  if (ticks > 8388607) ticks = 8388607;  // 23-bit max
  
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
  timer1_write(ticks);
  _enabled = true;
#endif
}
