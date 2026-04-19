// include/config.h
#ifndef CONFIG_H
#define CONFIG_H

// Serial communication settings
#define SERIAL_BAUD_RATE 115200

// Sensor calibration offsets (can be adjusted in production)
#define TEMPERATURE_OFFSET -2.5f  // Калибровка температуры
#define PRESSURE_OFFSET -10.0f   // Калибровка давления
#define HUMIDITY_OFFSET -3.0f    // Калибровка влажности


// Deep sleep interval in milliseconds (for battery operation)
#define DEEP_SLEEP_INTERVAL 60000  // 60 seconds

// Watchdog timer timeout in seconds
#define WATCHDOG_TIMEOUT 1000

// BME280 SPI sensor settings
// ESP8266 (Wemos D1 Mini) SPI pins:
// MOSI: D7 (GPIO13)
// MISO: D6 (GPIO12)
// SCK:  D5 (GPIO14) - SHARED with E-Ink display
// CS:   D4 (GPIO2) - Chip Select pin (D3/GPIO0 is FLASH button, not safe)
#define BME280_SPI_MOSI_PIN D7  // GPIO13 - MOSI pin
#define BME280_SPI_MISO_PIN D6  // GPIO12 - MISO pin
#define BME280_SPI_SCK_PIN  D5  // GPIO14 - SCK pin (shared with display)
#define BME280_SPI_CS_PIN   D3  // GPIO0  - Chip Select pin

// E-Ink Display connection info (Heltec 2.13" BWR)
// See display_config.h for detailed pin configuration
// Note: Display uses HSPI interface, shares CLK with BME280
// Display pins: RST=D1(GPIO5), DC=D2, CS=D8, BUSY=D0, CLK=D5

#endif // CONFIG_H