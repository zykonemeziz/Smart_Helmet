/*
 * drowsiness.cpp — IR-sensor drowsiness detection
 *
 * IR sensor output:
 *   LOW  = object detected (eyes closed / IR reflected)
 *   HIGH = no object (eyes open)
 *
 * If eyes stay closed ≥ DROWSY_THRESHOLD_MS (3 s):
 *   → Activate buzzer
 *   → Send SMS (once)
 *
 * When eyes reopen:
 *   → Stop buzzer
 *   → Reset SMS flag (so next event triggers again)
 */

#include "drowsiness.h"
#include "config.h"
#include "gps_handler.h"
#include "sms_handler.h"
#include "fall_detection.h"   // for getFallState() — avoid buzzer conflict

// ─── Internal state ─────────────────────────────────────────────────────────
static unsigned long eyesClosedStart   = 0;
static bool          eyesClosed        = false;
static bool          smsSentDrowsy     = false;
static bool          drowsyBuzzerOn    = false;

// ─── Main check (call every loop) ──────────────────────────────────────────
void checkDrowsiness() {
  unsigned long now = millis();
  bool irDetected = (digitalRead(IR_SENSOR_PIN) == LOW);   // LOW = eyes closed

  if (irDetected) {
    // ── Eyes closed ─────────────────────────────────────────────────────
    if (!eyesClosed) {
      eyesClosed      = true;
      eyesClosedStart = now;
    }

    unsigned long closedDuration = now - eyesClosedStart;

    if (closedDuration >= DROWSY_THRESHOLD_MS) {
      // Activate buzzer
      if (!drowsyBuzzerOn) {
        drowsyBuzzerOn = true;
        digitalWrite(BUZZER_PIN, HIGH);
        Serial.println("[DROWSY] >> Eyes closed too long — buzzer ON");
      }

      // Send SMS once per event
      if (!smsSentDrowsy) {
        String msg = "Emergency Alert! Drowsiness detected. Location: " + getGPSLocation();
        sendSMS(msg);
        smsSentDrowsy = true;
        Serial.println("[DROWSY] >> SMS queued");
      }
    }

  } else {
    // ── Eyes open ───────────────────────────────────────────────────────
    if (eyesClosed) {
      eyesClosed = false;

      if (drowsyBuzzerOn) {
        drowsyBuzzerOn = false;
        // Only turn off buzzer if fall countdown isn't also buzzing
        if (getFallState() != FALL_COUNTDOWN) {
          digitalWrite(BUZZER_PIN, LOW);
        }
        Serial.println("[DROWSY] >> Eyes open — buzzer OFF");
      }

      // Reset so a NEW drowsy event can trigger again
      smsSentDrowsy = false;
    }
  }
}
