# DIY Jobo Film Development Controller

ESP8266-based controller for a DIY rotary film processor (Jobo-style). Automates motor control, timing, and temperature monitoring for consistent film development.

## Features

- **Motor Control**: Adjustable RPM (1-80), soft ramping, auto-reverse
- **Multi-Step Timer**: Up to 10 development steps with pause between steps
- **Temperature Monitoring**: DS18B20 sensor with coefficient adjustment
- **Temperature Limits**: Alarm when temp goes outside min/max range
- **OLED Menu**: Full settings control via rotary encoder
- **Temperature Coefficient**: Auto-adjust timer or RPM based on temperature deviation

## Bill of Materials (BOM)

| Component | Description | Quantity |
|-----------|-------------|----------|
| NodeMCU v2 | ESP8266 development board | 1 |
| SSD1306 OLED | 128x64 I2C display | 1 |
| NEMA17 Stepper | 1.8° stepper motor | 1 |
| TMC2209 | Stepper driver (or A4988/DRV8825) | 1 |
| DS18B20 | Waterproof temperature sensor | 1 |
| Rotary Encoder | KY-040 with push button | 1 |
| Push Button | Momentary (for Back) | 1 |
| 4.7kΩ Resistor | Pull-up for DS18B20 | 1 |
| 12V Power Supply | For stepper motor | 1 |
| 5V Regulator | For ESP8266 (if not using USB) | 1 |

## Wiring (ESP8266 NodeMCU)

| Function | Pin | GPIO |
|----------|-----|------|
| STEP | D4 | GPIO2 |
| DIR | D8 | GPIO15 |
| Encoder CLK | D7 | GPIO13 |
| Encoder DT | D3 | GPIO0 |
| Encoder SW | D6 | GPIO12 |
| Back Button | D0 | GPIO16 |
| DS18B20 | D5 | GPIO14 |
| OLED SDA | D2 | GPIO4 |
| OLED SCL | D1 | GPIO5 |

## Building

Requires [PlatformIO](https://platformio.org/).

```bash
# Build
pio run -e esp8266

# Upload
pio run -e esp8266 -t upload

# Serial monitor
pio device monitor -b 115200
```

## Usage

1. **Main Screen**: Shows RPM, temperature, step progress
2. **Encoder rotation**: Adjust RPM on main screen
3. **Encoder press**: Enter settings menu
4. **OK button**: Start/stop, confirm selections
5. **Back button**: Stop process, exit menus

### Menu Structure

- **RPM**: Set target motor speed
- **Steps**: Configure development steps (duration each)
- **Reverse**: Enable auto-reverse and interval
- **Temperature**: Coefficient settings and alarm limits

## Temperature Coefficient

Automatically adjusts timer duration or RPM based on actual vs target temperature:

```
adjusted = base × (1 + (actualTemp - baseTemp) × percent / 100)
```

Example: At 22°C with base=20°C and percent=10%, timer runs 20% faster.

## License

MIT
