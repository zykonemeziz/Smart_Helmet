/*
 * gps_handler.cpp — NEO-6M GPS reading & location formatting
 *
 * Uses UART1 and TinyGPS++ to parse NMEA sentences.
 */

#include "gps_handler.h"
#include "config.h"
#include <TinyGPS++.h>
#include <HardwareSerial.h>

static TinyGPSPlus gps;
static HardwareSerial gpsSerial(1);  // UART1

void gpsSetup() {
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
}

void readGPS() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }
}

String getGPSLocation() {
  if (gps.location.isValid()) {
    return String(gps.location.lat(), 6) + ", " + String(gps.location.lng(), 6);
  }
  return "GPS not available";
}
