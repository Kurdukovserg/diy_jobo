#pragma once

// Minimal Arduino mock for native testing
#include <cstdint>
#include <cmath>
#include <cstdio>

// Arduino types
typedef uint8_t byte;

// Mock millis() - controllable for testing
static uint32_t _mockMillis = 0;
inline uint32_t millis() { return _mockMillis; }
inline void setMockMillis(uint32_t ms) { _mockMillis = ms; }
inline void advanceMockMillis(uint32_t ms) { _mockMillis += ms; }

// Mock micros()
inline uint32_t micros() { return _mockMillis * 1000; }

// Serial mock
struct SerialMock {
  void begin(int) {}
  void print(const char*) {}
  void println(const char*) {}
  void printf(const char*, ...) {}
};
static SerialMock Serial;

// Pin modes (no-op)
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define HIGH 1
#define LOW 0
inline void pinMode(uint8_t, uint8_t) {}
inline void digitalWrite(uint8_t, uint8_t) {}
inline int digitalRead(uint8_t) { return LOW; }
inline int analogRead(uint8_t) { return 0; }

// Interrupts (no-op)
inline void noInterrupts() {}
inline void interrupts() {}

// Tone (no-op)
inline void tone(uint8_t, unsigned int, unsigned long = 0) {}
inline void noTone(uint8_t) {}

// Delay (no-op for tests)
inline void delay(uint32_t) {}
inline void yield() {}
