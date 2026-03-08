# Wiring Diagram — Smart Helmet

Pin-by-pin wiring reference for all components.

## ESP32 Pin Map

```
                    ┌──────────────┐
                    │    ESP32     │
                    │              │
          SDA ──────│ GPIO 21      │
          SCL ──────│ GPIO 22      │──── MPU6050 (I2C)
                    │              │
     GPS TX ────────│ GPIO 16 (RX) │
     GPS RX ────────│ GPIO 17 (TX) │──── NEO-6M GPS (UART1)
                    │              │
     GSM TX ────────│ GPIO 26 (RX) │
     GSM RX ────────│ GPIO 27 (TX) │──── SIM800L (UART2)
                    │              │
   IR Sensor OUT ───│ GPIO 34      │──── IR Obstacle Sensor
                    │              │
        LED(+) ─────│ GPIO 2       │──── Red LED (via 220Ω resistor)
                    │              │
     Buzzer(+) ─────│ GPIO 25      │──── Active Buzzer
                    │              │
     Button ────────│ GPIO 4       │──── Push Button (pull-up)
                    │              │
                    │ 3.3V / 5V    │──── Power rails
                    │ GND          │──── Common ground
                    └──────────────┘
```

## Component Wiring Details

### MPU6050 (I2C Accelerometer/Gyroscope)

| MPU6050 Pin | ESP32 Pin | Notes |
|-------------|-----------|-------|
| VCC | 3.3V | **Do NOT use 5V** — module has no regulator |
| GND | GND | |
| SDA | GPIO 21 | Default I2C data |
| SCL | GPIO 22 | Default I2C clock |
| AD0 | GND | Sets I2C address to 0x68 |

### NEO-6M GPS (UART1)

| GPS Pin | ESP32 Pin | Notes |
|---------|-----------|-------|
| VCC | 3.3V or 5V | Check module — some need 5V |
| GND | GND | |
| TX | GPIO 16 | ESP32 UART1 RX |
| RX | GPIO 17 | ESP32 UART1 TX |

### SIM800L GSM (UART2)

| GSM Pin | ESP32 Pin | Notes |
|---------|-----------|-------|
| VCC | 3.7–4.2V | **Needs separate power** — draws up to 2A peaks |
| GND | GND | Common ground with ESP32 |
| TX | GPIO 26 | ESP32 UART2 RX |
| RX | GPIO 27 | ESP32 UART2 TX (via voltage divider 5V→3.3V) |

> ⚠️ SIM800L requires a dedicated power supply (e.g., 18650 LiPo cell or buck converter). Do NOT power from ESP32's 3.3V pin — it cannot supply enough current.

### IR Obstacle Sensor (Drowsiness Detection)

| IR Pin | ESP32 Pin | Notes |
|--------|-----------|-------|
| VCC | 3.3V | |
| GND | GND | |
| OUT | GPIO 34 | Input-only pin — **digital output: LOW = detected** |

### Red LED

| LED Pin | ESP32 Pin | Notes |
|---------|-----------|-------|
| Anode (+) | GPIO 2 | Via 220Ω current-limiting resistor |
| Cathode (−) | GND | |

### Active Buzzer

| Buzzer Pin | ESP32 Pin | Notes |
|------------|-----------|-------|
| Positive (+) | GPIO 25 | Direct drive (active buzzer, no PWM needed) |
| Negative (−) | GND | |

### Cancel Push Button

| Button Pin | ESP32 Pin | Notes |
|------------|-----------|-------|
| Terminal 1 | GPIO 4 | Internal pull-up enabled in code |
| Terminal 2 | GND | Button press = LOW |
