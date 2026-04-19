// src/sensor_manager.cpp
#include "sensor_manager.h"
#include "display_config.h"
#include "utils.h"
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_BME280.h>

SensorManager::SensorManager() : initialized(false), m_temperature(0.0f), m_pressure(0.0f), m_humidity(0.0f) {
    // Constructor
}

SensorManager::~SensorManager() {
    // Destructor
}

bool SensorManager::begin() {
    Serial.println("Initializing BME280 sensor...");
    setupSensor();
    checkSensorStatus();
    return initialized;
}

void SensorManager::update() {
    if (!initialized) return;

    // Read sensor data
    m_temperature = bme.readTemperature();
    m_pressure = bme.readPressure() / 100.0F;  // hPa
    m_humidity = bme.readHumidity();

    // Apply calibration offsets
    m_temperature += TEMPERATURE_OFFSET;
    m_pressure += PRESSURE_OFFSET;
    m_humidity += HUMIDITY_OFFSET;
}

void SensorManager::printData() {
    if (!initialized) return;

    // Format and print JSON message using string concatenation
    String message = "{\"timestamp\":\"";
    message += Utils::getTimestamp();
    message += "\",\"temperature\":";
    message += String(m_temperature, 1);
    message += ",\"pressure\":";
    message += String(m_pressure, 2);
    message += ",\"humidity\":";
    message += String(m_humidity, 1);
    message += ",\"status\":\"OK\"}";

    Utils::printJSONMessage(message);
}

float SensorManager::getTemperature() {
    if (!initialized) return 0.0f;
    return m_temperature;
}

float SensorManager::getPressure() {
    if (!initialized) return 0.0f;
    return m_pressure;
}

float SensorManager::getHumidity() {
    if (!initialized) return 0.0f;
    return m_humidity;
}

bool SensorManager::isInitialized() const {
    return initialized;
}

void SensorManager::setupSensor() {
      if (!bme.begin(BME280_SPI_CS_PIN)) {
        // SPI initialization failed
        initialized = false;
        Serial.println("BME280.begin() returned false!");
        Serial.println("Check wiring and power supply");
        return;
    }
    
    // Set the sensor parameters for optimal performance
    // MODE_NORMAL: Continuous measurement mode
    // SAMPLING_X2: Medium resolution for balance between accuracy and power
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,     // Operating mode
                    Adafruit_BME280::SAMPLING_X2,     // Temperature resolution
                    Adafruit_BME280::SAMPLING_X2,     // Pressure resolution
                    Adafruit_BME280::SAMPLING_X2);    // Humidity resolution

    initialized = true;
}

void SensorManager::checkSensorStatus() {
    // Check if the sensor is initialized
    if (!initialized) {
        Serial.println("BME280 sensor not initialized!");
        return;
    }

    // Check if the sensor is responding by reading temperature
    float temp = bme.readTemperature();
    
    // Feed watchdog to prevent reset
    yield();

    if (isnan(temp) || temp < -40.0f || temp > 85.0f) {
        Serial.println("BME280 sensor not responding!");
        initialized = false;
    } else {
        Serial.print("BME280 sensor initialized successfully! Temperature: ");
        Serial.print(temp);
        Serial.println(" °C");
    }
}