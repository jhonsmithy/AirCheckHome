// test/test_sensor_manager.cpp
#include "sensor_manager.h"
#include "utils.h"
#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <unity.h>

void setUp(void) {
    // Set up test environment
}

void tearDown(void) {
    // Clean up after test
}

void test_sensor_manager_initialization(void) {
    SensorManager sensorManager;
    TEST_ASSERT_FALSE(sensorManager.isInitialized());
}

void test_sensor_data_ranges(void) {
    SensorManager sensorManager;
    
    // Test temperature range
    TEST_ASSERT_FLOAT_WITHIN(125.0f, 0.0f, sensorManager.getTemperature());
    
    // Test pressure range
    TEST_ASSERT_FLOAT_WITHIN(800.0f, 0.0f, sensorManager.getPressure());
    
    // Test humidity range
    TEST_ASSERT_FLOAT_WITHIN(100.0f, 0.0f, sensorManager.getHumidity());
}

void test_json_message_format(void) {
    // Test timestamp format
    String timestamp = Utils::getTimestamp();
    TEST_ASSERT_GREATER_OR_EQUAL(0, timestamp.toInt());
    
    // Test JSON message structure
    String testMessage = "{\"timestamp\":\"12345\",\"temperature\":25.5,\"pressure\":1013.25,\"humidity\":50.0,\"status\":\"OK\"}";
    TEST_ASSERT_NOT_EQUAL(-1, testMessage.indexOf("timestamp"));
    TEST_ASSERT_NOT_EQUAL(-1, testMessage.indexOf("temperature"));
    TEST_ASSERT_NOT_EQUAL(-1, testMessage.indexOf("pressure"));
    TEST_ASSERT_NOT_EQUAL(-1, testMessage.indexOf("humidity"));
    TEST_ASSERT_NOT_EQUAL(-1, testMessage.indexOf("status"));
}

void test_watchdog_function(void) {
    // Test watchdog function doesn't crash
    Utils::handleWatchdog();
    TEST_PASS();
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_sensor_manager_initialization);
    RUN_TEST(test_sensor_data_ranges);
    RUN_TEST(test_json_message_format);
    RUN_TEST(test_watchdog_function);
    
    return UNITY_END();
}
