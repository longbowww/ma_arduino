#ifndef LEDMANAGER_H
#define LEDMANAGER_H

#include <Adafruit_NeoPixel.h>

class LEDManager {
public:
    LEDManager(Adafruit_NeoPixel& strip);
    void setup();
    void updateStatus(bool wifiConnected, bool mqttConnected, bool canConnected);

private:
    Adafruit_NeoPixel& strip_;
};

#endif // LEDMANAGER_H