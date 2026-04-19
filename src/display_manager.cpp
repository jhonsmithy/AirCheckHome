// src/display_manager.cpp
#include "display_manager.h"
#include "config.h"
#include <Arduino.h>

#if DISPLAY_ENABLED
#include "heltec-eink-modules.h"
#endif

DisplayManager::DisplayManager() : initialized(false) {
#if DISPLAY_ENABLED
    epd = nullptr;
#endif
}

DisplayManager::~DisplayManager() {
    sleep();
#if DISPLAY_ENABLED
    if (epd) {
        delete epd;
    }
#endif
}

bool DisplayManager::begin() {
#if DISPLAY_ENABLED
    Serial.println("Configuring pins for SPI conflict prevention...");
    
    // CRITICAL: Ensure BME280 CS is HIGH (not selected) to avoid SPI conflict
    pinMode(BME280_SPI_CS_PIN, OUTPUT);
    digitalWrite(BME280_SPI_CS_PIN, HIGH);
    delay(20); // Longer delay for pin stabilization

    // CRITICAL: E-Ink CS must be HIGH before any SPI communication
    // GPIO15 (D8) is pulled LOW at boot on ESP8266, so we MUST set it HIGH immediately
    pinMode(EINK_CS_PIN, OUTPUT);
    digitalWrite(EINK_CS_PIN, HIGH);
    delay(20); // Longer delay for pin stabilization
    
    // Verify CS pins are actually HIGH and diagnose if not
    Serial.printf("E-Ink CS pin state: %s (should be HIGH)\n", 
                  digitalRead(EINK_CS_PIN) == HIGH ? "HIGH" : "LOW");
    Serial.printf("BME280 CS pin state: %s (should be HIGH)\n", 
                  digitalRead(BME280_SPI_CS_PIN) == HIGH ? "HIGH" : "LOW");
    
    if (digitalRead(EINK_CS_PIN) == LOW) {
        Serial.println("ERROR: E-Ink CS pin stuck LOW!");
        Serial.println("Attempting to force HIGH...");
        digitalWrite(EINK_CS_PIN, HIGH);
        delay(100);
        if (digitalRead(EINK_CS_PIN) == LOW) {
            Serial.println("ERROR: Cannot set E-Ink CS HIGH!");
            return false;
        }
    }
    if (digitalRead(BME280_SPI_CS_PIN) == LOW) {
        Serial.println("ERROR: BME280 CS pin stuck LOW!");
        return false;
    }
    
    Serial.println("SPI CS pins configured correctly");
    
    // Check BUSY pin status before initialization
    Serial.printf("BUSY pin initial state: %s (should be LOW for ready)\n", 
                  digitalRead(EINK_BUSY_PIN) == HIGH ? "HIGH (busy)" : "LOW (ready)");
    Serial.println("Initializing E-Ink display...");

    // Create display object with pins: DC, CS, BUSY
    // RST pin is set separately, SPI is already initialized in main.cpp
    Serial.printf("Display pins: DC=%d, CS=%d, BUSY=%d, RST=%d\n", 
                  EINK_DC_PIN, EINK_CS_PIN, EINK_BUSY_PIN, EINK_RST_PIN);
    
    epd = new QYEG0213RWS800(EINK_DC_PIN, EINK_CS_PIN, EINK_BUSY_PIN);

    if (!epd) {
        Serial.println("Failed to allocate display object!");
        return false;
    }

    // Configure RST pin - MUST be done before display.begin()
    pinMode(EINK_RST_PIN, OUTPUT);
    
    // Perform proper reset sequence with longer delays
    Serial.println("Performing display reset sequence...");
    digitalWrite(EINK_RST_PIN, LOW);
    delay(50);
    digitalWrite(EINK_RST_PIN, HIGH);
    delay(100);

    // Ensure CS is still HIGH after reset
    digitalWrite(EINK_CS_PIN, HIGH);
    delay(20);

    // Check BUSY pin status BEFORE initialization
    Serial.printf("BUSY pin state before begin(): %s\n", 
                  digitalRead(EINK_BUSY_PIN) == HIGH ? "HIGH (busy)" : "LOW (ready)");
    
    // Initialize display - library uses existing SPI bus
    Serial.println("Calling epd->begin()...");
    epd->begin();
    Serial.println("epd->begin() completed!");

    // Check BUSY pin status after initialization
    Serial.printf("BUSY pin state after begin(): %s\n", 
                  digitalRead(EINK_BUSY_PIN) == HIGH ? "HIGH (busy)" : "LOW (ready)");
    
    // Wait for display to become ready with longer timeout
    Serial.println("Waiting for display to become ready...");
    unsigned long busy_start = millis();
    const unsigned long BUSY_TIMEOUT = 20000; // 20 seconds timeout
    
    int retry_count = 0;
    while (digitalRead(EINK_BUSY_PIN) == HIGH) {
        if (millis() - busy_start > BUSY_TIMEOUT) {
            Serial.println("ERROR: BUSY pin timeout - display stuck busy");
            Serial.println("Trying to force display reset...");
            retry_count++;
            if (retry_count <= 2) {
                // Force reset if stuck
                digitalWrite(EINK_RST_PIN, LOW);
                delay(100);
                digitalWrite(EINK_RST_PIN, HIGH);
                delay(200);
                Serial.printf("Retry %d - checking BUSY state again...\n", retry_count);
                continue;
            } else {
                Serial.println("ERROR: Multiple reset attempts failed!");
                return false;
            }
        }
        delay(100); // Longer delay for stability
        yield();
    }
    Serial.println("Display is now ready");
    Serial.printf("Final BUSY state: %s\n", 
                  digitalRead(EINK_BUSY_PIN) == HIGH ? "HIGH (busy)" : "LOW (ready)");
    
    // CRITICAL: Ensure CS is HIGH before clear operation
    digitalWrite(EINK_CS_PIN, HIGH);
    delay(20);

    // Clear display using fillRect
    Serial.println("Clearing display with fillRect...");
    epd->fillRect(0, 0, epd->width(), epd->height(), WHITE);
    
    // Update display with proper timeout handling
    Serial.println("Updating display...");
    unsigned long update_start = millis();
    epd->update();
    
    // Wait for update to complete with longer timeout
    while (digitalRead(EINK_BUSY_PIN) == HIGH) {
        if (millis() - update_start > BUSY_TIMEOUT) {
            Serial.println("ERROR: Update timeout - display stuck busy");
            return false;
        }
        delay(50); // Longer delay
        yield();
    }
    Serial.println("Display update completed");

    initialized = true;
    Serial.println("E-Ink display initialized successfully!");
    return true;
#else
    Serial.println("Display disabled in config");
    initialized = false;
    return false;
#endif
}

void DisplayManager::showSensorData(float temperature, float pressure, float humidity, bool sensorOk) {
#if DISPLAY_ENABLED
    if (!initialized || !epd) {
        Serial.println("Display not initialized!");
        return;
    }

    // CRITICAL: Manage SPI bus arbitration between BME280 and E-Ink display
    // 1. De-select BME280 (CS HIGH = disabled) to prevent bus conflicts
    digitalWrite(BME280_SPI_CS_PIN, HIGH);
    delay(1); // Small delay to ensure BME280 CS is stable

    // 2. E-Ink display CS is managed by the library
    // Clear screen with white rectangle
    epd->fillRect(0, 0, epd->width(), epd->height(), WHITE);

    // Draw border
    epd->drawRect(0, 0, epd->width(), epd->height(), BLACK);

    // Draw header "AirCheck"
    epd->setCursor(10, 20);
    epd->setTextColor(BLACK);
    epd->setTextSize(2);
    epd->print("AirCheck");

    // Draw separator line
    epd->drawLine(5, 40, 115, 40, BLACK);

    // Format sensor data strings
    String tempStr = "T: " + String(temperature, 1) + " C";
    String presStr = "P: " + String(pressure, 1) + " hPa";
    String humStr = "H: " + String(humidity, 1) + " %";

    // Draw sensor data
    int yPos = 60;
    epd->setCursor(10, yPos);
    epd->setTextColor(BLACK);
    epd->setTextSize(1);
    epd->print(tempStr);

    epd->setCursor(10, yPos + 20);
    epd->print(presStr);

    epd->setCursor(10, yPos + 40);
    epd->print(humStr);

    // Update display (this sends data to E-Ink via SPI) with timeout handling
    Serial.println("Updating display with sensor data...");
    Serial.printf("BUSY state before update: %s\n", 
                  digitalRead(EINK_BUSY_PIN) == HIGH ? "HIGH (busy)" : "LOW (ready)");
    
    unsigned long update_start = millis();
    const unsigned long BUSY_TIMEOUT = 15000; // 15 seconds timeout
    
    epd->update();
    
    // Wait for update to complete with timeout
    while (digitalRead(EINK_BUSY_PIN) == HIGH) {
        if (millis() - update_start > BUSY_TIMEOUT) {
            Serial.println("ERROR: Sensor data update timeout - display stuck busy");
            // Try to recover by checking BUSY pin state
            Serial.printf("Final BUSY state: %s\n", 
                          digitalRead(EINK_BUSY_PIN) == HIGH ? "HIGH (busy)" : "LOW (ready)");
            return;
        }
        delay(50); // Longer delay for stability
        yield();
    }
    Serial.printf("BUSY state after update: %s\n", 
                  digitalRead(EINK_BUSY_PIN) == HIGH ? "HIGH (busy)" : "LOW (ready)");
    Serial.println("Display update with sensor data completed");

    // 3. After update, E-Ink CS is automatically de-selected by library
    // BME280 can now safely use SPI bus again

    Serial.println("Display updated with sensor data");
#else
    // Display disabled - just log
    (void)temperature;
    (void)pressure;
    (void)humidity;
    (void)sensorOk;
#endif
}

void DisplayManager::showStatus(const String& status) {
#if DISPLAY_ENABLED
    if (!initialized || !epd) {
        Serial.println("Display not initialized!");
        return;
    }

    Serial.printf("showStatus() called with: '%s'\n", status.c_str());

    // CRITICAL: De-select BME280 (CS HIGH = disabled) to prevent bus conflicts
    digitalWrite(BME280_SPI_CS_PIN, HIGH);
    delay(1); // Small delay for stability

    // Clear screen
    epd->fillRect(0, 0, epd->width(), epd->height(), WHITE);

    // Draw status text centered
    epd->setCursor(10, epd->height() / 2);
    epd->setTextColor(BLACK);
    epd->setTextSize(2);
    epd->print(status);

    // Update display with timeout handling
    Serial.println("Updating display with status...");
    unsigned long update_start = millis();
    const unsigned long BUSY_TIMEOUT = 10000; // 10 seconds timeout
    
    epd->update();
    
    // Wait for update to complete with timeout
    while (digitalRead(EINK_BUSY_PIN) == HIGH) {
        if (millis() - update_start > BUSY_TIMEOUT) {
            Serial.println("ERROR: Status update timeout - display stuck busy");
            return;
        }
        delay(10);
        yield();
    }
    Serial.println("Display status update completed");

    Serial.println("Display updated successfully!");
#else
    (void)status;
#endif
}

void DisplayManager::showTestMessage() {
#if DISPLAY_ENABLED
    if (!initialized || !epd) {
        Serial.println("Display not initialized!");
        return;
    }

    Serial.println("showTestMessage() - displaying test message");

    // CRITICAL: De-select BME280 (CS HIGH = disabled) to prevent bus conflicts
    digitalWrite(BME280_SPI_CS_PIN, HIGH);
    delay(1);

    // Clear screen
    epd->fillRect(0, 0, epd->width(), epd->height(), WHITE);

    // Draw border
    epd->drawRect(0, 0, epd->width(), epd->height(), BLACK);

    // Draw header
    epd->setCursor(10, 25);
    epd->setTextColor(BLACK);
    epd->setTextSize(2);
    epd->print("AirCheck");

    // Draw separator
    epd->drawLine(5, 45, 115, 45, BLACK);

    // Draw test message
    epd->setCursor(10, 70);
    epd->setTextColor(BLACK);
    epd->setTextSize(1);
    epd->print("BME280: OFF");

    epd->setCursor(10, 90);
    epd->print("Display: OK");

    epd->setCursor(10, 110);
    epd->print("TEST MODE");

    // Draw red test rectangle (if display supports it)
    epd->fillRect(10, 130, 50, 10, RED);

    // Update display with timeout handling
    Serial.println("Updating display with test message...");
    unsigned long update_start = millis();
    const unsigned long BUSY_TIMEOUT = 10000; // 10 seconds timeout
    
    epd->update();
    
    // Wait for update to complete with timeout
    while (digitalRead(EINK_BUSY_PIN) == HIGH) {
        if (millis() - update_start > BUSY_TIMEOUT) {
            Serial.println("ERROR: Test message update timeout - display stuck busy");
            return;
        }
        delay(10);
        yield();
    }
    Serial.println("Display test message update completed");

    Serial.println("Test message displayed successfully!");
#else
    Serial.println("Display disabled - cannot show test message");
#endif
}

void DisplayManager::sleep() {
#if DISPLAY_ENABLED
    if (initialized && epd) {
        // Display doesn't have explicit sleep in new library
        // Could potentially power down if needed
    }
#endif
}

void DisplayManager::drawText(int x, int y, const String& text, bool black) {
#if DISPLAY_ENABLED
    // This method is deprecated - use epd directly in showSensorData/showStatus
    (void)x;
    (void)y;
    (void)text;
    (void)black;
#endif
}

void DisplayManager::drawLargeText(int x, int y, const String& text, bool black) {
#if DISPLAY_ENABLED
    // This method is deprecated - use epd directly in showSensorData/showStatus
    (void)x;
    (void)y;
    (void)text;
    (void)black;
#endif
}

void DisplayManager::clearScreen() {
#if DISPLAY_ENABLED
    if (!initialized || !epd) return;

    // CRITICAL: De-select BME280 (CS HIGH = disabled) to prevent bus conflicts
    digitalWrite(BME280_SPI_CS_PIN, HIGH);
    delay(1);

    // Skip clear() to avoid BUSY pin hang
    // Just do a landscape reset
    //epd->landscape();
    //epd->clear(); // COMMENTED OUT
    // epd->update(); // COMMENTED OUT
    // Корректная очистка:
    epd->fillRect(0, 0, epd->width(), epd->height(), WHITE);
    epd->update();
#endif
}
