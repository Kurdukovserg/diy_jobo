#pragma once
#include <Arduino.h>

// Device
constexpr const char* DEVICE_NAME = "DIY-JOBO";

// WiFi AP fallback
constexpr const char* WIFI_AP_SSID = "DIY-JOBO";
constexpr const char* WIFI_AP_PASS = "12345678";

// Ports
constexpr uint16_t HTTP_PORT = 80;
constexpr uint16_t WS_PORT   = 81;
constexpr uint16_t DISCOVERY_PORT = 45454;

// ---------- PIN DEFINITIONS ----------
#if defined(ESP32)
  // ESP32 DevKit pins (adjust to your wiring)
  constexpr uint8_t PIN_STEP = 17;
  constexpr uint8_t PIN_DIR  = 16;
  constexpr uint8_t PIN_ENC_A    = 34;
  constexpr uint8_t PIN_ENC_B    = 35;
  constexpr uint8_t PIN_ENC_SW   = 39;  // encoder push button (confirm/menu)
  constexpr uint8_t PIN_BTN_OK   = 32;
  constexpr uint8_t PIN_BTN_BACK = 33;
  constexpr uint8_t PIN_DS18B20  = 4;
  constexpr uint8_t PIN_SDA = 21;
  constexpr uint8_t PIN_SCL = 22;
#else
  // ESP8266 NodeMCU pins
  constexpr uint8_t PIN_STEP = D4;   // GPIO2
  constexpr uint8_t PIN_DIR  = D8;   // GPIO15
  constexpr uint8_t PIN_ENC_A    = D7;   // encoder CLK
  constexpr uint8_t PIN_ENC_B    = D3;   // encoder DT
  constexpr uint8_t PIN_ENC_SW   = D6;   // encoder push button (was showing as OK)
  constexpr uint8_t PIN_BTN_OK   = 3;    // GPIO3 (RX) - OK button (or unused)
  constexpr uint8_t PIN_BTN_BACK = D0;   // BACK button (stop/menu/exit)
  constexpr uint8_t PIN_DS18B20 = D5;
  constexpr uint8_t PIN_SDA = D2;
  constexpr uint8_t PIN_SCL = D1;
#endif

// Motor parameters (NEMA17 + TMC2209)
constexpr int STEPS_PER_REV = 200;     // 1.8° stepper
constexpr int MICROSTEPS    = 16;      // TMC2209 microstepping
constexpr int STEPS_EFF     = STEPS_PER_REV * MICROSTEPS; // 3200 steps/rev

// RPM limits (NEMA17 can go faster than 28BYJ)
constexpr int RPM_MIN = 1;
constexpr int RPM_MAX = 80;

// ---------- TIMING ----------
constexpr uint32_t ENC_DEBOUNCE_US = 50;  // reduced for better responsiveness
constexpr uint16_t UI_UPDATE_MS    = 100;   // OLED refresh rate
constexpr uint16_t TEMP_PERIOD_MS  = 1000;  // temp read cycle
constexpr uint16_t TEMP_CONV_MS    = 800;   // DS18B20 conversion time
constexpr uint16_t UI_FPS_MS = 50;
