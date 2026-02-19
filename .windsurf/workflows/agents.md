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
   - Settings: RPM, reverse, temp coefficient (profile & per-step)
   - Alarm actions: None, Beep, Pause, Stop
   - States: running, paused (between steps), stopped

3. **MenuController** (`MenuController.h/cpp`)
   - Handles all menu navigation and editing
   - Screen state machine
   - Submenus: Profile (Custom > Settings/Steps), Reverse, Buzzer, Hardware

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

9. **BuzzerController** (`BuzzerController.h/cpp`)
   - Software PWM buzzer control
   - Alert types: StepFinished, ProcessEnded, TempWarning

## Menu Structure

```
SETTINGS (Main Menu):
├── Profile >
│   └── Custom >                    ← Profile selection (future: Default, Profile1, etc.)
│       ├── Settings >              ← Profile-level temp coef settings
│       │   ├── TempCoef: ON/OFF
│       │   ├── Base: 20.0°C
│       │   ├── Percent: 10%
│       │   ├── Target: Timer/RPM/Both
│       │   ├── Alarm: None/Beep/Pause/Stop
│       │   ├── Limits: ON/OFF
│       │   ├── Min: 18.0°C
│       │   └── Max: 24.0°C
│       └── Steps (N) >             ← Step list
│           └── Step N >            ← Step detail (6 items)
│               ├── Duration: M:SS
│               ├── RPM: 30
│               ├── Temp: OFF/20.0°C
│               ├── Bias: OFF/+/-2.0°C
│               ├── Name: (optional)
│               └── TempCoef: Profile/Custom >  ← Per-step override
├── Reverse >
│   ├── Enabled: ON/OFF
│   └── Interval: 10s
├── Buzzer >
│   ├── Enabled: ON/OFF
│   ├── Step Finished: ON/OFF
│   ├── Process Ended: ON/OFF
│   ├── Temp Warning: ON/OFF
│   ├── Frequency: 2200Hz
│   └── Test
└── Hardware >
    ├── Steps/Rev: 200
    ├── Microsteps: 16
    ├── Driver: TMC2209/A4988/DRV8825
    ├── Motor Invert: ON/OFF
    ├── Buzzer Type: Active/Passive
    ├── Buzzer Active High: ON/OFF
    └── Temp Offset: 0.0°C
```

## Data Structures

### ProcessStep
```cpp
struct ProcessStep {
  int32_t durationSec = 0;        // 0 = disabled
  int32_t rpm = 30;               // RPM for this step
  StepTempMode tempMode;          // Off, Target
  float tempTarget = 20.0f;       // target temp if enabled
  StepTempBiasMode tempBiasMode;  // Off, Bias
  float tempBias = 2.0f;          // +/- tolerance
  char name[STEP_NAME_LEN];       // step name (empty = "Step N")
  
  // Per-step temp coefficient override
  bool tempCoefOverride = false;  // if true, use step-specific settings
  bool tempCoefEnabled = false;
  float tempCoefBase = 20.0f;
  float tempCoefPercent = 10.0f;
  TempCoefTarget tempCoefTarget;
  TempAlarmAction tempAlarmAction;
};
```

### SessionSettings
```cpp
struct SessionSettings {
  int32_t targetRpm = 30;
  char profileName[PROFILE_NAME_LEN];
  
  bool reverseEnabled = true;
  float reverseIntervalSec = 10.0f;
  
  ProcessStep steps[MAX_STEPS];
  int8_t stepCount = 1;
  
  // Profile-level temperature coefficient
  bool tempCoefEnabled = false;
  float tempCoefBase = 20.0f;
  float tempCoefPercent = 10.0f;
  TempCoefTarget tempCoefTarget;   // Timer, Rpm, Both
  TempAlarmAction tempAlarmAction; // None, Beep, Pause, Stop
  
  // Temperature limits
  bool tempLimitsEnabled = false;
  float tempMin = 18.0f;
  float tempMax = 24.0f;
};
```

### Enums
```cpp
enum class TempCoefTarget : uint8_t { Timer, Rpm, Both };
enum class TempAlarmAction : uint8_t { None, Beep, Pause, Stop };
enum class StepTempMode : uint8_t { Off, Target };
enum class StepTempBiasMode : uint8_t { Off, Bias };
```

## Current Features

### ✅ Implemented
- Motor speed control (1-80 RPM)
- Soft RPM ramping
- Auto-reverse with configurable interval
- Multi-step development profile (up to 10 steps)
- Per-step settings: duration, RPM, temp target, temp bias, name
- Per-step temp coefficient override (use profile or custom settings)
- Pause between steps (wait for user OK)
- Add/edit/delete steps via menu
- Temperature monitoring (DS18B20)
- Temperature coefficient (profile-level and per-step override)
- Temperature coefficient affects: Timer (shorter time at higher temp), RPM, or Both
- Temperature limits with alarm actions (None, Beep, Pause, Stop)
- Profile menu structure (prepared for multiple profiles)
- OLED menu system with submenus
- Rotary encoder navigation
- Multiple button inputs with debouncing
- Buzzer alerts (step finished, process ended, temp warning)
- Hardware settings menu (driver type, microsteps, etc.)

---

# Implementation Plan - Next Steps

## Phase 1: Complete Core Features ✅ DONE

### 1.1 Temperature Coefficient Application ✅
- [x] Apply tempCoef to timer duration
- [x] Apply tempCoef to RPM  
- [x] Per-step temp coefficient override
- [x] Show adjusted values on main screen

### 1.2 Per-Step Parameters ✅
- [x] Per-step RPM
- [x] Per-step temperature target
- [x] Per-step temperature bias
- [x] Per-step name
- [x] Per-step temp coefficient override

### 1.3 Alarm Actions ✅
- [x] TempAlarmAction enum (None, Beep, Pause, Stop)
- [x] Profile-level alarm action setting
- [x] Per-step alarm action override
- [x] Pause/Stop process on temp limit

### 1.4 Beeper/Buzzer Support ✅
- [x] Buzzer controller with software PWM
- [x] Beep on step completion
- [x] Beep pattern for session complete
- [x] Temperature warning beeps
- [x] Buzzer menu with settings

## Phase 2: Persistence

### 2.1 Settings Storage
- [ ] Save settings to LittleFS
- [ ] Load settings on boot
- [ ] Auto-save on change (with debounce)

### 2.2 Development Profiles
- [ ] Save multiple named profiles
- [ ] Profile selection in ProfileMenu (currently only "Custom")
- [ ] Default profiles (C-41, B&W, E-6)

## Phase 3: Enhanced UI

### 3.1 Main Screen Improvements
- [x] Show current step name
- [x] Big step indicator
- [ ] Progress bar for current step
- [ ] Show adjusted time when tempCoef active

### 3.2 Better Step Editing
- [x] Step name/label
- [x] Per-step RPM
- [ ] Copy/reorder steps

## Phase 4: Connectivity (Optional)

### 4.1 WiFi Features
- [ ] Web interface for settings
- [ ] OTA firmware updates
- [ ] WebSocket for real-time status

## Phase 5: Advanced Features

### 5.1 Temperature Control
- [ ] Water bath temperature target
- [ ] Heater relay control
- [ ] PID temperature regulation

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
- `STEP_NAME_LEN = 12` - Max chars for step name
- `PROFILE_NAME_LEN = 16` - Max chars for profile name
- `RPM_MIN = 1, RPM_MAX = 80` - RPM limits
- `STARTUP_IGNORE_MS = 500` - Input settling delay
