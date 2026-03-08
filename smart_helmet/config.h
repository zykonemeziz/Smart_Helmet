/*
 * config.h — Pin definitions, thresholds, and timing constants
 *
 * Edit this file to match your wiring and tuning preferences.
 * Every other file includes this, so changes propagate everywhere.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ─── Pin Definitions ────────────────────────────────────────────────────────
#define BUZZER_PIN      25
#define LED_PIN         2
#define BUTTON_PIN      4
#define IR_SENSOR_PIN   34    // Input-only GPIO — confirm your wiring

// UART pins
#define GPS_RX_PIN      16
#define GPS_TX_PIN      17
#define GSM_RX_PIN      26
#define GSM_TX_PIN      27

// ─── Phone Number ───────────────────────────────────────────────────────────
// Replace with the real emergency contact number (include country code)
#define PHONE_NUMBER    "+918891246434"

// ─── Fall Detection Thresholds ──────────────────────────────────────────────
#define IMPACT_THRESHOLD       20.0f   // m/s² — total accel to trigger fall
#define NORMAL_THRESHOLD       12.0f   // m/s² — below this = "normal" orientation

// ─── Timing (milliseconds) ─────────────────────────────────────────────────
#define COUNTDOWN_MS           5000UL  // Full alert countdown duration
#define LED_BLINK_START_MS     3000UL  // LED starts blinking at this point into countdown
#define LED_BLINK_INTERVAL_MS  500UL   // LED on/off toggle interval
#define NORMAL_HOLD_MS         2000UL  // Sustained normal accel before reset
#define DROWSY_THRESHOLD_MS    3000UL  // Eyes closed this long = drowsy
#define ACCEL_READ_INTERVAL_MS 300UL   // MPU6050 polling interval

#endif // CONFIG_H
