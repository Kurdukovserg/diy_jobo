---
description: DIY Jobo Film Development Controller - Project Context and Implementation Plan
---

# DIY Jobo Film Development Controller

## Project Overview

ESP8266-based controller for a DIY Jobo-style film development rotary processor. Controls motor speed, direction reversal, temperature monitoring, and multi-step development timing.

## Hardware

- **MCU**: ESP8266 (NodeMCU v2)
- **Display**: SSD1306 128x64 OLED (I2C)
- **Motor Driver**: Stepper motor with step/dir interface
- **Temperature Sensor**: DS18B20 one-wire
- **Input**: Rotary encoder with button, OK button, Back button (analog A0)

### Pin Configuration (from Config.h)
- `PIN_STEP`, `PIN_DIR` - Stepper motor control
- `PIN_SDA`, `PIN_SCL` - I2C for OLED
- `PIN_DS18B20` - Temperature sensor
- `PIN_ENC_A`, `PIN_ENC_B`, `PIN_ENC_SW` - Rotary encoder
- `PIN_OK`, `PIN_BACK` - Buttons
- A0 - Analog back button

## Architecture

### Core Classes

1. **App** (`App.h/cpp`)
   - Main coordinator
   - Initializes all subsystems
   - Routes input to MenuController
   - Updates UiModel from session/menu state

2. **SessionController** (`SessionController.h/cpp`)
   - Manages motor and development session
   - Multi-step timer with pause between steps
   - Settings: RPM, reverse, temp coefficient
   - States: running, paused (between steps), stopped

3. **MenuController** (`MenuController.h/cpp`)
   - Handles all menu navigation and editing
   - Screen state machine
   - Submenus: Steps, Reverse, TempCoef

4. **MotorController** (`MotorController.h/cpp`)
   - Soft RPM ramping
   - Direction reversal state machine
   - Interfaces with StepperISR

5. **StepperISR** (`StepperISR.h/cpp`)
   - Timer-based interrupt for step pulses
   - Low-level motor control

6. **Ui** (`Ui.h/cpp`)
   - OLED display rendering
   - Uses UiModel for state
   - Multiple draw methods for each screen

7. **Inputs** (`Inputs.h/cpp`)
   - Debounced input handling
   - Encoder, buttons, analog input
   - Startup ignore delay for button settling

8. **TempSensor** (`TempSensor.h/cpp`)
   - DS18B20 temperature reading
   - Non-blocking async reads

## Data Structures

### ProcessStep
```cpp
struct ProcessStep {
  int32_t durationSec = 0;  // 0 = disabled
};
```

### SessionSettings
```cpp
struct SessionSettings {
  int32_t targetRpm = 30;
  bool reverseEnabled = true;
  float reverseIntervalSec = 10.0f;
  ProcessStep steps[MAX_STEPS];  // MAX_STEPS = 10
  int8_t stepCount = 1;
  bool tempCoefEnabled = false;
  float tempCoefBase = 20.0f;
  float tempCoefPercent = 10.0f;
  TempCoefTarget tempCoefTarget;  // Timer, Rpm, Both
};
```

### Screen States
```cpp
enum class Screen : uint8_t {
  Main, Menu, EditRpm,
  StepsMenu, EditStepDuration,
  ReverseMenu, EditReverseEnabled, EditReverseInterval,
  TempCoefMenu, EditTempCoefEnabled, EditTempCoefBase, 
  EditTempCoefPercent, EditTempCoefTarget
};
```

## Current Features

### ✅ Implemented
- Motor speed control (1-80 RPM)
- Soft RPM ramping
- Auto-reverse with configurable interval
- Multi-step development profile (up to 10 steps)
- Each step has independent duration
- Pause between steps (wait for user OK)
- Add/edit/delete steps via menu
- Temperature monitoring (DS18B20)
- Temperature coefficient display
- Temperature coefficient settings (base, percent, target)
- OLED menu system with submenus
- Rotary encoder navigation
- Multiple button inputs with debouncing
- Startup input ignore (500ms settling)

### 🔄 Partially Implemented
- ~~Temperature coefficient calculation~~ ✅ Now fully implemented with temp limits alarm
- ~~Step durations visible in menu~~ ✅ Now shows actual durations

---

# Implementation Plan - Next Steps

## Phase 1: Complete Core Features

### 1.1 Temperature Coefficient Application
- [x] Apply tempCoef to timer duration when `TempCoefTarget::Timer` or `Both`
- [x] Apply tempCoef to RPM when `TempCoefTarget::Rpm` or `Both`
- [x] Formula: `adjusted = base * (1 + (tempC - tempCoefBase) * tempCoefPercent / 100)`
- [x] Show adjusted values on main screen

### 1.2 Fix Steps Menu Display
- [x] Pass steps durations array to UiModel
- [x] Display actual step durations in steps menu

### 1.3 Beeper/Buzzer Support
- [ ] Add buzzer pin to Config.h
- [ ] Beep on step completion
- [ ] Beep pattern for session complete
- [ ] Optional beep on button press

## Phase 2: Persistence

### 2.1 Settings Storage
- [ ] Save settings to LittleFS
- [ ] Load settings on boot
- [ ] Auto-save on change (with debounce)

### 2.2 Development Profiles
- [ ] Save multiple named profiles
- [ ] Profile selection menu
- [ ] Default profiles (C-41, B&W, E-6)

## Phase 3: Enhanced UI

### 3.1 Main Screen Improvements
- [ ] Show total remaining time
- [ ] Progress bar for current step
- [ ] Show adjusted time when tempCoef active

### 3.2 Better Step Editing
- [ ] Step name/label (optional)
- [ ] Per-step RPM override (optional)
- [ ] Copy/reorder steps

## Phase 4: Connectivity (Optional)

### 4.1 WiFi Features
- [ ] Web interface for settings
- [ ] OTA firmware updates
- [ ] WebSocket for real-time status

### 4.2 Remote Control
- [ ] Start/stop via web
- [ ] Mobile-friendly UI

## Phase 5: Advanced Features

### 5.1 Temperature Control
- [ ] Water bath temperature target
- [ ] Heater relay control
- [ ] PID temperature regulation

### 5.2 Pre-soak/Agitation Patterns
- [ ] Configurable agitation patterns per step
- [ ] Initial agitation burst
- [ ] Periodic agitation intervals

---

## Quick Reference

### Build Commands
```bash
# Build
pio run -e esp8266

# Upload
pio run -e esp8266 -t upload

# Monitor serial
pio device monitor -b 115200
```

### File Locations
- Headers: `/include/`
- Sources: `/src/`
- Config: `/include/Config.h`
- PlatformIO: `/platformio.ini`

### Key Constants
- `MAX_STEPS = 10` - Maximum development steps
- `RPM_MIN = 1, RPM_MAX = 80` - RPM limits
- `STARTUP_IGNORE_MS = 500` - Input settling delay
