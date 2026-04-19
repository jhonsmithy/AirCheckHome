// src/display_manager.cpp
#include "display_manager.h"
#include "config.h"
#include <Arduino.h>

DisplayManager::DisplayManager() : initialized(false) {
    epd = nullptr;
}

DisplayManager::~DisplayManager() {
    sleep();
    if (epd) {
        delete epd;
    }
}

bool DisplayManager::begin() {
    // CRITICAL: Ensure BME280 CS is HIGH (not selected) to avoid SPI conflict
    pinMode(BME280_SPI_CS_PIN, OUTPUT);
    digitalWrite(BME280_SPI_CS_PIN, HIGH);
    delay(100); // Longer delay for pin stabilization

    epd = new QYEG0213RWS800(EINK_DC_PIN, EINK_CS_PIN, EINK_BUSY_PIN);

    if (!epd) {
        Serial.println("Failed to allocate display object!");
        return false;
    }
    epd->begin();
    // Clear display using fillRect
    Serial.println("Clearing display with fillRect...");
    epd->clear();
    
    // epd->fillRect(0, 0, epd->width(), epd->height(), WHITE);
    //epd->update();

    initialized = true;
    return initialized;
}

void DisplayManager::showSensorData(float temperature, float pressure, float humidity, bool sensorOk) {
    if (!initialized || !epd) {
        Serial.println("Display not initialized!");
        return;
    }

    // CRITICAL: Manage SPI bus arbitration between BME280 and E-Ink display
    // 1. De-select BME280 (CS HIGH = disabled) to prevent bus conflicts
    pinMode(BME280_SPI_CS_PIN, OUTPUT);
    digitalWrite(BME280_SPI_CS_PIN, HIGH);
    delay(10); // Small delay to ensure BME280 CS is stable

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
    delay(100);
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
}

void DisplayManager::showStatus(const String& status) {
    if (!initialized || !epd) {
        Serial.println("Display not initialized!");
        return;
    }

    Serial.printf("showStatus() called with: '%s'\n", status.c_str());

    // CRITICAL: De-select BME280 (CS HIGH = disabled) to prevent bus conflicts
    pinMode(BME280_SPI_CS_PIN, OUTPUT);
    digitalWrite(BME280_SPI_CS_PIN, HIGH);
    delay(10); // Small delay for stability
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

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
    SPI.endTransaction();
    
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
}

void DisplayManager::showTestMessage() {
    if (!initialized || !epd) {
        Serial.println("Display not initialized!");
        return;
    }

    Serial.println("showTestMessage() - displaying test message");

    // CRITICAL: De-select BME280 (CS HIGH = disabled) to prevent bus conflicts
    pinMode(BME280_SPI_CS_PIN, OUTPUT);
    digitalWrite(BME280_SPI_CS_PIN, HIGH);
    pinMode(EINK_CS_PIN, OUTPUT);
    digitalWrite(EINK_CS_PIN, LOW);
    delay(10);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

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
    delay(2000);
    SPI.endTransaction();

    // Wait for update to complete with timeout
    while (digitalRead(EINK_BUSY_PIN) == HIGH) {
        if (millis() - update_start > BUSY_TIMEOUT) {
            Serial.println("ERROR: Test message update timeout - display stuck busy");
            return;
        }
        delay(10);
        yield();
    }

    Serial.println("Test message displayed successfully!");
}

void DisplayManager::sleep() {
    if (initialized && epd) {
        // Display doesn't have explicit sleep in new library
        // Could potentially power down if needed
    }
}

void DisplayManager::drawText(int x, int y, const String& text, bool black) {
    // This method is deprecated - use epd directly in showSensorData/showStatus
    (void)x;
    (void)y;
    (void)text;
    (void)black;
}

void DisplayManager::clearScreen() {
    if (!initialized || !epd) return;

    // CRITICAL: De-select BME280 (CS HIGH = disabled) to prevent bus conflicts
    pinMode(BME280_SPI_CS_PIN, OUTPUT);
    digitalWrite(BME280_SPI_CS_PIN, HIGH);
    delay(10);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

    // Just do a landscape reset
    //epd->landscape();
    //epd->clear(); // COMMENTED OUT
    // Корректная очистка:
    epd->fillRect(0, 0, epd->width(), epd->height(), WHITE);
    epd->update();
    SPI.endTransaction();
}
