// include/display_config.h
#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

// E-Ink Display Pin Configuration (Heltec 2.13" BWR)
// Display      ESP8266 Pin     Note
// VCC          3.3V            ⚠️ Only 3.3V, not 5V!
// GND          GND             Common ground
// CLK          GPIO14 (D5)     HSPI SCK
// DIN/MOSI     GPIO13 (D7)     HSPI MOSI
// CS           GPIO15 (D8)     HSPI CS, ⚠️ pulled to GND at boot
// DC           GPIO4 (D2)      Any free digital pin
// RST          GPIO5 (D1)      Any free digital pin
// BUSY         GPIO16 (D0)     Supports deep sleep wake-up

#define EINK_RST_PIN    D1    // GPIO5  - Reset pin
#define EINK_DC_PIN     D2    // GPIO4  - Data/Command pin
#define EINK_CS_PIN     D8    // GPIO15 - Chip Select pin
#define EINK_BUSY_PIN   D0    // GPIO16 - Busy pin
#define EINK_CLK_PIN    D5    // GPIO14 - Clock pin (HSPI)
// MOSI is D7 (GPIO13) - using default HSPI MOSI

// Display dimensions
// Heltec 2.13" BWR: 250 pixels wide x 122 pixels tall in landscape orientation
// But for Paint class: width is the shorter dimension (byte-aligned), height is longer
#define EINK_WIDTH      122
#define EINK_HEIGHT     250

// Display update interval (in milliseconds)
// E-Ink displays should not be updated more than once per minute
#define DISPLAY_UPDATE_INTERVAL  60000  // 60 seconds (same as measurement interval)

// Enable/disable display
#define DISPLAY_ENABLED   false

#endif // DISPLAY_CONFIG_H
