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

    // CRITICAL: BME280 library handles its own CS pin management
    // CS is automatically set LOW during SPI transactions and HIGH after
    // Ensure E-Ink display CS is HIGH (not selected) to prevent bus conflicts
    pinMode(EINK_CS_PIN, OUTPUT);
    digitalWrite(EINK_CS_PIN, HIGH);

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
    // Set BME280 CS pin HIGH initially (device not selected)
    pinMode(BME280_SPI_CS_PIN, OUTPUT);
    digitalWrite(BME280_SPI_CS_PIN, HIGH);

    // Set E-Ink display CS pin HIGH to prevent it from interfering with SPI bus
    pinMode(EINK_CS_PIN, OUTPUT);
    digitalWrite(EINK_CS_PIN, HIGH);

    // Delay to allow pins to stabilize and prevent boot conflicts
    delay(10);

    // Initialize the BME280 sensor via hardware SPI
    // NOTE: SPI bus is already initialized in main.cpp
    // The Adafruit_BME280 library will use the existing SPI bus
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

    // Ensure E-Ink display CS is HIGH (not selected) to prevent bus conflicts
    pinMode(EINK_CS_PIN, OUTPUT);
    digitalWrite(EINK_CS_PIN, HIGH);

    // Check if the sensor is responding
    // For SPI, we can read the sensor ID register
    // The BME280 has a chip ID register at 0xD0
    // We'll try to read temperature to verify the sensor is working
    float temp = bme.readTemperature();
    
    // Feed watchdog to prevent reset
    yield();

    if (isnan(temp) || temp < -40.0f || temp > 85.0f) {
        Serial.println("BME280 sensor not responding!");
        initialized = false;
    } else {
        Serial.println("BME280 sensor initialized successfully!");
    }
}