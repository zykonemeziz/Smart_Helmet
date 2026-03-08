/*
 * gps_handler.h — GPS module interface
 */

#ifndef GPS_HANDLER_H
#define GPS_HANDLER_H

#include <Arduino.h>

/// Call once in setup() to initialise GPS UART
void gpsSetup();

/// Call every loop() iteration to feed bytes to TinyGPS++ parser
void readGPS();

/// Returns "lat, lng" string or "GPS not available"
String getGPSLocation();

#endif // GPS_HANDLER_H
