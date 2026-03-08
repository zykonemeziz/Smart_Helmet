# Smart Helmet System — ESP32

Fall detection, drowsiness alert, and emergency SMS notification system built on ESP32.

## Hardware

| Component | Interface | Default Pins |
|-----------|-----------|-------------|
| MPU6050 (accelerometer) | I2C | SDA 21 / SCL 22 |
| NEO-6M GPS | UART1 | RX 16 / TX 17 |
| SIM800L GSM | UART2 | RX 26 / TX 27 |
| IR Sensor (eye detection) | Digital | GPIO 34 |
| Red LED | Digital | GPIO 2 |
| Buzzer | Digital | GPIO 25 |
| Cancel Button | Digital (pull-up) | GPIO 4 |

## Project Structure

```
Smart_Helmet/
├── README.md                              ← You are here
├── docs/
│   └── wiring_diagram.md                  ← Pin-by-pin wiring reference
└── smart_helmet/                          ← Arduino project folder
    ├── smart_helmet.ino                   ← Main sketch
    ├── config.h                           ← Pins & constants
    ├── fall_detection.h / .cpp            ← Fall detection
    ├── drowsiness.h / .cpp                ← Drowsiness detection
    ├── gps_handler.h / .cpp               ← GPS parsing
    └── sms_handler.h / .cpp               ← SMS handling
```

## Features

- **Fall Detection** — MPU6050 impact → 5s countdown → SMS with GPS
  - LED solid for first 3s, blinks at 500ms interval for remaining 2s
  - Cancel button aborts alert at any point during countdown
  - System resets when helmet returns to normal orientation for 2s
- **Drowsiness Detection** — IR sensor eyes-closed ≥ 3s → buzzer + SMS
  - Buzzer stops immediately when eyes reopen
  - SMS flag resets so next event triggers again
- **Non-blocking** — all timing via `millis()`, both features run concurrently
- **SMS Spam Prevention** — flags prevent duplicate alerts per event

## Quick Start

1. Install **Arduino IDE** with ESP32 board support
2. Install libraries:
   - `Adafruit MPU6050`
   - `Adafruit Unified Sensor`
   - `TinyGPSPlus`
3. Open `smart_helmet/config.h` and set:
   - `PHONE_NUMBER` — your emergency contact (with country code)
   - Pin numbers if your wiring differs from defaults
4. Open `smart_helmet/smart_helmet.ino` → Select your ESP32 board → **Upload**

## Serial Monitor

Open at **115200 baud**. Log prefixes for easy filtering:
- `[FALL]` — fall detection state machine events
- `[DROWSY]` — drowsiness detection events
- `[SMS]` — GSM AT-command steps

## License

MIT
