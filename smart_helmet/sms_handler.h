/*
 * sms_handler.h — SIM800L SMS sending interface
 */

#ifndef SMS_HANDLER_H
#define SMS_HANDLER_H

#include <Arduino.h>

/// Call once in setup() to initialise GSM UART
void smsSetup();

/// Queue an SMS for non-blocking sending. If already sending, the message is dropped.
void sendSMS(const String& message);

/// Call every loop() iteration to advance the AT-command state machine
void processSMSQueue();

/// Returns true if an SMS is currently being sent
bool isSMSSending();

#endif // SMS_HANDLER_H
