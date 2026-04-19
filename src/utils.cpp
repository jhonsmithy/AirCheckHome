// src/utils.cpp
#include "utils.h"
#include "config.h"     // Для доступа к WATCHDOG_TIMEOUT
#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h> // Для доступа к ESP классу
#include <ets_sys.h>     // Для доступа к системным функциям watchdog

// Print a formatted JSON message to Serial
void Utils::printJSONMessage(const String& message) {
    Serial.println(message);
}

// Format a timestamp in Unix epoch time
String Utils::getTimestamp() {
    // Return Unix epoch time in seconds
    // For ESP8266, we can use millis() divided by 1000 to get seconds
    // In a real IoT application, you might want to sync with NTP for accurate time
    // For now, using system time since startup
    return String(millis() / 1000);
}

// Handle watchdog timer
void Utils::handleWatchdog() {
    // ESP8266 hardware watchdog timer implementation
    // The ESP8266 has a hardware watchdog timer that resets the system if not fed
    // This function should be called periodically to prevent watchdog timeout
    
    // Enable hardware watchdog timer
    // ESP8266 watchdog timeout is in microseconds (1000000 = 1 second)
    // We set it to WATCHDOG_TIMEOUT seconds from config.h
    wdt_enable(WATCHDOG_TIMEOUT * 1000000UL);
    
    // Reset the watchdog timer
    wdt_reset();
}
