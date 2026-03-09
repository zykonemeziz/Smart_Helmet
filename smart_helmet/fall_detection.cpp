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
#include "drowsiness.h"

// ─── Internal state ─────────────────────────────────────────────────────────
static Adafruit_MPU6050 mpu;

static FallState     fallState          = FALL_MONITORING;
static unsigned long countdownStart     = 0;
static bool          smsSentFall        = false;
static unsigned long lastAccelRead      = 0;
static float         lastTotalAccel     = 0;
static float         lastTiltAngle      = 0;
static bool          tiltWarningActive  = false;

// Reset-to-normal tracking
static unsigned long normalStartTime    = 0;
static bool          normalTimerRunning = false;

// LED blink helper
static bool          ledBlinkOn         = false;
static unsigned long lastBlinkToggle    = 0;

// Debug: avoid printing the same countdown second twice
static unsigned long lastPrintedSec     = 0xFFFFFFFF;

// Tilt blink helper
static bool          tiltLedOn          = false;
static unsigned long lastTiltBlinkToggle = 0;

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

  // Tilt calculation (in degrees)
  // tilt = atan2(ax, sqrt(ay*ay + az*az)) * 180 / PI
  lastTiltAngle = atan2(a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180.0f / M_PI;
  lastTiltAngle = abs(lastTiltAngle); // We want the magnitude (either direction)

  tiltWarningActive = (lastTiltAngle >= TILT_THRESHOLD_DEG);

  // Trigger only in monitoring state
  if (fallState == FALL_MONITORING && lastTotalAccel > IMPACT_THRESHOLD) {
    Serial.println("[FALL] >> Impact detected!");
    fallState          = FALL_COUNTDOWN;
    countdownStart     = now;
    smsSentFall        = false;
    ledBlinkOn         = false;
    normalTimerRunning = false;
    lastPrintedSec     = 0xFFFFFFFF;
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
        fallState = FALL_MONITORING;
        return;
      }

      // LED behaviour
      if (elapsed >= LED_BLINK_START_MS) {
        // 3–5 s : blink
        if (now - lastBlinkToggle >= LED_BLINK_INTERVAL_MS) {
          ledBlinkOn = !ledBlinkOn;
          lastBlinkToggle = now;
        }
      }
      // else: LED already solid ON from trigger

      // Countdown expired → send SMS
      if (elapsed >= COUNTDOWN_MS) {
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

    default:
      break;
  }
}

void updateIndicatorsPriority() {
  unsigned long now = millis();
  bool finalLedState = false;
  bool finalBuzzerState = false;

  // 1. FALL DETECTION (Highest Priority)
  if (fallState == FALL_COUNTDOWN) {
    unsigned long elapsed = now - countdownStart;
    finalBuzzerState = true; // Buzzer solid during countdown
    if (elapsed < LED_BLINK_START_MS) {
      finalLedState = true; // Solid ON 0-3s
    } else {
      // Blink 3-5s
      finalLedState = ledBlinkOn;
    }
  } 
  // 2. DROWSINESS (Middle Priority)
  else if (getDrowsyActive()) {
    finalBuzzerState = true;
    finalLedState = false; // Drowsiness doesn't use LED in current spec
  }
  // 3. TILT WARNING (Lowest Priority)
  else if (tiltWarningActive) {
    // Handle tilt blinking timing
    if (now - lastTiltBlinkToggle >= TILT_BLINK_INTERVAL_MS) {
      tiltLedOn = !tiltLedOn;
      lastTiltBlinkToggle = now;
    }
    finalLedState = tiltLedOn;
    finalBuzzerState = false;
  }
  else {
    // Normal state
    finalLedState = false;
    finalBuzzerState = false;
  }

  // Apply to pins
  digitalWrite(LED_PIN,    finalLedState ? HIGH : LOW);
  digitalWrite(BUZZER_PIN, finalBuzzerState ? HIGH : LOW);
}

// ─── Public query ───────────────────────────────────────────────────────────
FallState getFallState() {
  return fallState;
}

float getTiltAngle() {
  return lastTiltAngle;
}

bool getTiltWarningActive() {
  return tiltWarningActive;
}
