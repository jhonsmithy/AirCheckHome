// include/display_manager.h
#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "display_config.h"
#include <Arduino.h>

#if DISPLAY_ENABLED
#include "heltec-eink-modules.h"
#endif

class DisplayManager {
public:
    DisplayManager();
    ~DisplayManager();

    bool begin();
    void showSensorData(float temperature, float pressure, float humidity, bool sensorOk);
    void showStatus(const String& status);
    void showTestMessage();  // NEW: Test message display
    void sleep();

private:
#if DISPLAY_ENABLED
    QYEG0213RWS800* epd;
#endif
    bool initialized;

    void drawText(int x, int y, const String& text, bool black = true);
    void drawLargeText(int x, int y, const String& text, bool black = true);
    void clearScreen();
};

#endif // DISPLAY_MANAGER_H
