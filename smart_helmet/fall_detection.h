/*
 * fall_detection.h — MPU6050 fall/impact detection interface
 */

#ifndef FALL_DETECTION_H
#define FALL_DETECTION_H

#include <Arduino.h>

/// Possible states of the fall-detection state machine
enum FallState {
  FALL_MONITORING,     // Normal — watching for impact
  FALL_COUNTDOWN,      // Impact detected — countdown running
  FALL_ALERT_SENT      // SMS sent — waiting for helmet to return to normal
};

/// Call once in setup() to initialise MPU6050
void fallSetup();

/// Read accelerometer periodically and detect impact (call every loop)
void checkFall();

/// Run the countdown / LED-blink / SMS state machine (call every loop)
void handleAlertState();

/// Returns the current fall-detection state
FallState getFallState();

#endif // FALL_DETECTION_H
