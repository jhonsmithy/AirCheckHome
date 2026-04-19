// include/sensor_manager.h
#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "config.h"
#include <Adafruit_BME280.h>

class SensorManager {
public:
    SensorManager();
    ~SensorManager();

    bool begin();
    void update();
    void printData();

    float getTemperature();
    float getPressure();
    float getHumidity();

    bool isInitialized() const;

private:
    Adafruit_BME280 bme;
    bool initialized;
    
    // Private member variables to store sensor readings
    float m_temperature;
    float m_pressure;
    float m_humidity;
    
    // Private helper methods
    void setupSensor();
    void checkSensorStatus();
};

#endif // SENSOR_MANAGER_H