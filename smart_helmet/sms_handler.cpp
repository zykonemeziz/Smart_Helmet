/*
 * sms_handler.cpp — Non-blocking SMS sending via SIM800L
 *
 * Uses a 4-step state machine to send AT commands with timed gaps
 * so that loop() is never blocked.
 *
 * Step 0: AT+CMGF=1           (text mode)       — wait 1 s
 * Step 1: AT+CMGS="<number>"  (recipient)        — wait 1 s
 * Step 2: <message body>                          — wait 500 ms
 * Step 3: Ctrl+Z (0x1A)       (send)             — wait 3 s
 */

#include "sms_handler.h"
#include "config.h"
#include <HardwareSerial.h>

static HardwareSerial gsmSerial(2);  // UART2

static bool          smsSending      = false;
static int           smsStep         = 0;
static unsigned long smsStepTime     = 0;
static String        smsPendingMsg   = "";

// ─── Setup ──────────────────────────────────────────────────────────────────
void smsSetup() {
  gsmSerial.begin(9600, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
}

// ─── Queue a message ────────────────────────────────────────────────────────
void sendSMS(const String& message) {
  if (smsSending) {
    Serial.println("   [SMS] Busy — dropped: " + message);
    return;
  }
  smsPendingMsg = message;
  smsSending    = true;
  smsStep       = 0;
  smsStepTime   = millis();

  gsmSerial.println("AT+CMGF=1");
  Serial.println("   [SMS] Step 0 → AT+CMGF=1");
}

// ─── Advance state machine (call every loop) ───────────────────────────────
void processSMSQueue() {
  if (!smsSending) return;

  unsigned long now = millis();

  switch (smsStep) {

    case 0:   // wait 1 s after CMGF
      if (now - smsStepTime >= 1000) {
        gsmSerial.println(String("AT+CMGS=\"") + PHONE_NUMBER + "\"");
        Serial.println("   [SMS] Step 1 → AT+CMGS");
        smsStep     = 1;
        smsStepTime = now;
      }
      break;

    case 1:   // wait 1 s for prompt, send body
      if (now - smsStepTime >= 1000) {
        gsmSerial.print(smsPendingMsg);
        Serial.println("   [SMS] Step 2 → body");
        smsStep     = 2;
        smsStepTime = now;
      }
      break;

    case 2:   // wait 500 ms, send Ctrl+Z
      if (now - smsStepTime >= 500) {
        gsmSerial.write(26);
        Serial.println("   [SMS] Step 3 → Ctrl+Z");
        smsStep     = 3;
        smsStepTime = now;
      }
      break;

    case 3:   // wait 3 s for network, done
      if (now - smsStepTime >= 3000) {
        Serial.println("   [SMS] ✓ Sent!");
        smsSending    = false;
        smsStep       = 0;
        smsPendingMsg = "";
      }
      break;
  }
}

// ─── Status query ───────────────────────────────────────────────────────────
bool isSMSSending() {
  return smsSending;
}
