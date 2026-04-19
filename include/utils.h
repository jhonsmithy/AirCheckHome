// include/utils.h
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <Adafruit_BME280.h>
#include "config.h"     // Для доступа к WATCHDOG_TIMEOUT

// Utility functions for the project
namespace Utils {
    // Function to print a formatted JSON message to Serial
    void printJSONMessage(const String& message);
    
    // Function to format a timestamp in Unix epoch time
    String getTimestamp();
    
    // Function to handle watchdog timer
    void handleWatchdog();
}

#endif // UTILS_H