// include/display_manager.h
#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "display_config.h"
#include <Arduino.h>
#include "heltec-eink-modules.h"


class DisplayManager {
public:
    DisplayManager();
    ~DisplayManager();

    bool begin();
    void showSensorData(float temperature, float pressure, float humidity, bool sensorOk);
    void showStatus(const String& status);
    void showTestMessage();  // NEW: Test message display
    void sleep();
    void clearScreen();
    
private:
    QYEG0213RWS800* epd;
    bool initialized;
    void drawText(int x, int y, const String& text, bool black = true);
    void drawLargeText(int x, int y, const String& text, bool black = true);
    
};

#endif // DISPLAY_MANAGER_H
