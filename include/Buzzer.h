#pragma once
#include <Arduino.h>

enum class BuzzerPattern : uint8_t {
  None = 0,
  Click,        // Single short beep (button press)
  StepDone,     // Two short beeps (step completed)
  SessionDone,  // Long beep + short beeps (session finished)
  Alarm         // Repeating beeps (temperature alarm)
};

class Buzzer {
public:
  void begin(uint8_t pin);
  void tick();
  
  void play(BuzzerPattern pattern);
  void stop();
  
  bool isPlaying() const { return _playing; }
  void setEnabled(bool enabled) { _enabled = enabled; }
  bool isEnabled() const { return _enabled; }

private:
  uint8_t _pin = 0;
  bool _enabled = true;
  bool _playing = false;
  bool _buzzerOn = false;
  
  BuzzerPattern _pattern = BuzzerPattern::None;
  uint8_t _step = 0;
  uint32_t _stepStartMs = 0;
  
  void setBuzzer(bool on);
  void advancePattern();
};
