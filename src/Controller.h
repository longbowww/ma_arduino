#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "WiFiManager.h"
#include "MQTTManager.h"
#include "CANManager.h"

struct Vehicle {
    int number;
    uint16_t speed;
    uint16_t distance;

    Vehicle(int num) : number(num), speed(0), distance(0) {}
};

class Controller {
public:
    Controller(int vehicleNum);
    void setup();
    void loop();

private:
    void processCANMessages();
    void publishVehicleData();

    WiFiManager wifiManager_;
    MQTTManager mqttManager_;
    CANManager canManager_;
    Vehicle vehicle_;
    String mqttClientId_;
};

#endif // CONTROLLER_H
