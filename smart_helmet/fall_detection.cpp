/*
 * fall_detection.cpp — MPU6050 fall/impact detection + alert countdown
 *
 * State machine:
 *   FALL_MONITORING  ──(accel > threshold)──▸  FALL_COUNTDOWN
 *   FALL_COUNTDOWN   ──(cancel button)──────▸  FALL_MONITORING
 *   FALL_COUNTDOWN   ──(5 s elapsed)────────▸  FALL_ALERT_SENT  (SMS sent)
 *   FALL_ALERT_SENT  ──(normal accel 2 s)───▸  FALL_MONITORING
 *
 * LED behaviour during COUNTDOWN:
 *   0–3 s : solid ON
 *   3–5 s : blink at 500 ms interval
 */

#include "fall_detection.h"
#include "config.h"
#include "gps_handler.h"
#include "sms_handler.h"
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

// ─── Internal state ─────────────────────────────────────────────────────────
static Adafruit_MPU6050 mpu;

static FallState     fallState          = FALL_MONITORING;
static unsigned long countdownStart     = 0;
static bool          smsSentFall        = false;
static unsigned long lastAccelRead      = 0;
static float         lastTotalAccel     = 0;

// Reset-to-normal tracking
static unsigned long normalStartTime    = 0;
static bool          normalTimerRunning = false;

// LED blink helper
static bool          ledBlinkOn         = false;
static unsigned long lastBlinkToggle    = 0;

// Debug: avoid printing the same countdown second twice
static unsigned long lastPrintedSec     = 0xFFFFFFFF;

// ─── Setup ──────────────────────────────────────────────────────────────────
void fallSetup() {
  if (!mpu.begin()) {
    Serial.println("[FALL] ERROR: MPU6050 not found!");
    while (1) { delay(10); }   // halt — sensor is mandatory
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("[FALL] MPU6050 initialised");
}

// ─── Read accelerometer (non-blocking, polled) ─────────────────────────────
void checkFall() {
  unsigned long now = millis();
  if (now - lastAccelRead < ACCEL_READ_INTERVAL_MS) return;
  lastAccelRead = now;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  lastTotalAccel = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );

  // Trigger only in monitoring state
  if (fallState == FALL_MONITORING && lastTotalAccel > IMPACT_THRESHOLD) {
    Serial.println("[FALL] >> Impact detected!");
    fallState          = FALL_COUNTDOWN;
    countdownStart     = now;
    smsSentFall        = false;
    ledBlinkOn         = false;
    normalTimerRunning = false;
    lastPrintedSec     = 0xFFFFFFFF;

    digitalWrite(LED_PIN,    HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
  }
}

// ─── Alert state machine ────────────────────────────────────────────────────
void handleAlertState() {
  unsigned long now = millis();

  switch (fallState) {

    // ── COUNTDOWN (5 s) ──────────────────────────────────────────────────
    case FALL_COUNTDOWN: {
      unsigned long elapsed = now - countdownStart;

      // Cancel button
      if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println("[FALL] >> Alert CANCELLED");
        digitalWrite(LED_PIN,    LOW);
        digitalWrite(BUZZER_PIN, LOW);
        fallState = FALL_MONITORING;
        return;
      }

      // LED behaviour
      if (elapsed >= LED_BLINK_START_MS) {
        // 3–5 s : blink
        if (now - lastBlinkToggle >= LED_BLINK_INTERVAL_MS) {
          ledBlinkOn = !ledBlinkOn;
          digitalWrite(LED_PIN, ledBlinkOn ? HIGH : LOW);
          lastBlinkToggle = now;
        }
      }
      // else: LED already solid ON from trigger

      // Countdown expired → send SMS
      if (elapsed >= COUNTDOWN_MS) {
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED_PIN,    LOW);

        if (!smsSentFall) {
          String msg = "Emergency Alert! Fall detected. Location: " + getGPSLocation();
          sendSMS(msg);
          smsSentFall = true;
          Serial.println("[FALL] >> SMS queued");
        }
        fallState = FALL_ALERT_SENT;
      }

      // Debug countdown
      unsigned long secLeft = (COUNTDOWN_MS - elapsed) / 1000;
      if (secLeft != lastPrintedSec) {
        lastPrintedSec = secLeft;
        Serial.print("[FALL]    Alert in ");
        Serial.print(secLeft + 1);
        Serial.println("s — press button to cancel");
      }
      break;
    }

    // ── ALERT SENT — wait for normal orientation ─────────────────────────
    case FALL_ALERT_SENT: {
      if (lastTotalAccel < NORMAL_THRESHOLD) {
        if (!normalTimerRunning) {
          normalTimerRunning = true;
          normalStartTime    = now;
        } else if (now - normalStartTime >= NORMAL_HOLD_MS) {
          Serial.println("[FALL] >> Helmet normal — resetting");
          fallState          = FALL_MONITORING;
          smsSentFall        = false;
          normalTimerRunning = false;
        }
      } else {
        normalTimerRunning = false;
      }
      break;
    }

    default:
      break;
  }
}

// ─── Public query ───────────────────────────────────────────────────────────
FallState getFallState() {
  return fallState;
}
