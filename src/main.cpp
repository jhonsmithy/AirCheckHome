// src/main.cpp
#include "sensor_manager.h"
#include "display_manager.h"
#include "utils.h"
#include "config.h"
#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <SPI.h>

// Global instances
SensorManager sensorManager;
DisplayManager displayManager;

// Non-blocking timing variables
unsigned long lastMeasurementTime = 0;
const unsigned long measurementInterval = DEEP_SLEEP_INTERVAL; // 60 seconds

void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD_RATE);

    // Small delay for serial initialization (non-blocking)
    delay(50);

    Serial.println("=== AirCheck Environment Monitor ===");
    Serial.println("Initializing...");
     // BME280
    SPI.pins(BME280_SPI_SCK_PIN, BME280_SPI_MISO_PIN, BME280_SPI_MOSI_PIN, BME280_SPI_CS_PIN);
    SPI.begin();
    Serial.println("Initializing SPI bus...");
    Serial.println("SPI bus initialized successfully!");

   // Initialize E-Ink display (always try to init display)
    Serial.println("About to initialize E-Ink display...");
    if (!displayManager.begin()) {
        Serial.println("E-Ink display initialization failed!");
    } else {
        Serial.println("E-Ink display initialized successfully!");
    }

    //displayManager.showTestMessage();

    // Initialize BME280 sensor via SPI (uses already initialized SPI bus)
    if (!sensorManager.begin()) {
        Serial.println("WARNING: BME280 sensor not initialized!");
    } else {
        Serial.println("BME280 sensor initialized successfully!");
    }

    // Initialize watchdog timer
    Utils::handleWatchdog();
    
    Serial.println("Starting measurements...");
    
    // Initialize timing
    lastMeasurementTime = millis();
}

void loop() {
    unsigned long currentTime = millis();

    // Non-blocking measurement and display loop
    if (currentTime - lastMeasurementTime >= measurementInterval) {
        Serial.println("=== Starting measurement cycle ===");
        
        // Update sensor data
        Serial.println("Updating sensor data...");
        sensorManager.update();

        // Print sensor data
        sensorManager.printData();

        // Update E-Ink display with sensor data
        Serial.println("About to update display...");
        if (sensorManager.isInitialized()) {
            Serial.println("Sensor OK, showing data...");
            
            displayManager.showSensorData(
                sensorManager.getTemperature(),
                sensorManager.getPressure(),
                sensorManager.getHumidity(),
                true
            );
            

           //displayManager.clearScreen();
           //displayManager.showTestMessage();

        } else {
            // Show status when sensor is not connected
            Serial.println("Sensor not initialized, showing 'No Sensor'...");
            displayManager.showStatus("No Sensor");
        }
        
        Serial.println("Measurement cycle completed!");

        // Reset watchdog timer
        Utils::handleWatchdog();

        // Update last measurement time
        lastMeasurementTime = currentTime;
    }

    // Small delay to prevent overwhelming the serial output
    delay(1); // Minimal delay for stability
}