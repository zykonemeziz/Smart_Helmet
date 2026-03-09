/*
 * drowsiness.h — IR-sensor drowsiness detection interface
 */

#ifndef DROWSINESS_H
#define DROWSINESS_H

#include <Arduino.h>

/// Call every loop() iteration to check eye-closure duration
void checkDrowsiness();

/// Returns true if drowsiness alert (buzzer/SMS) is currently active
bool getDrowsyActive();

#endif // DROWSINESS_H
