/*
 * smart_helmet.ino — Main sketch (thin orchestrator)
 *
 * This file only handles setup() and loop().
 * All logic lives in the module files:
 *
 *   config.h               — Pins, thresholds, timing constants
 *   gps_handler.h/.cpp     — NEO-6M GPS parsing
 *   sms_handler.h/.cpp     — SIM800L non-blocking SMS sending
 *   fall_detection.h/.cpp  — MPU6050 impact detection + countdown
 *   drowsiness.h/.cpp      — IR sensor eye-closure detection
 *
 * To debug a specific feature, open only its .cpp file.
 * To change pins or thresholds, edit config.h.
 */

#include "config.h"
#include "gps_handler.h"
#include "sms_handler.h"
#include "fall_detection.h"
#include "drowsiness.h"

void setup() {
  Serial.begin(115200);

  // GPIO
  pinMode(BUZZER_PIN,    OUTPUT);
  pinMode(LED_PIN,       OUTPUT);
  pinMode(BUTTON_PIN,    INPUT_PULLUP);
  pinMode(IR_SENSOR_PIN, INPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN,    LOW);

  // Subsystems
  gpsSetup();
  smsSetup();
  fallSetup();

  Serial.println("=== Smart Helmet System Ready ===");
}

void loop() {
  readGPS();           // Feed GPS parser
  checkFall();         // Read MPU6050, detect impact
  handleAlertState();  // Run fall-alert countdown / SMS
  checkDrowsiness();   // Read IR sensor, detect drowsiness
  processSMSQueue();   // Advance non-blocking SMS sending
}
