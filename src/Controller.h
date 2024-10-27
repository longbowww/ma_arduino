#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "MQTTManager.h"
#include "CANManager.h"
#include "LEDManager.h"
#include "WiFiManager.h"
#include <Arduino.h>

// Vehicle struct definition
struct Vehicle
{
    int number;
    float speed;
    float distance;

    Vehicle(int num) : number(num), speed(0.0), distance(0.0) {}
};

// Controller class definition
class Controller
{
public:
    Controller(int vehicleNum);
    void setup();
    void loop();

private:
       // CAN message constants
    static constexpr unsigned long CAN_ID_DATA = 0x69;    // For vehicle data
    static constexpr unsigned long CAN_ID_CONTROL = 0x66; // For control commands
    static constexpr unsigned char MIN_MESSAGE_LENGTH = 4;
    static constexpr uint16_t SPEED_SCALE_FACTOR = 100;   // For converting float to int
            // Constants for topic patterns
    static constexpr const char* TOPIC_PATTERN_COMMAND = "/vehicle/%d/command";
    static constexpr const char* TOPIC_PATTERN_DATA = "/vehicle/%d/data";
    // Helper method to format topics
    String formatTopic(const char* pattern, int vehicleNumber) const;

    // functional methods for data transmission
    void setupSubscriptions();
    void processCANMessages();
    void publishVehicleData(const unsigned char* buf, unsigned char len);
    void onReceivedMessage(const String& topic, const String& payload);

    WiFiManager wifiManager_;
    MQTTManager mqttManager_;
    CANManager canManager_;

    Vehicle vehicle_;

    // Topic handlers
    void handleSpeedCommand(uint8_t speed);
    void handleVehicleRelay(int sourceVehicleId, const String& payload);
    
    // Helper methods
    bool sendCANMessage(unsigned long canId, const unsigned char* data, unsigned char length);
    int extractVehicleId(const String& topic) const;
    bool isCommandTopic(const String& topic) const { return topic.indexOf("/command") != -1; }
    bool isDataTopic(const String& topic) const { return topic.indexOf("/data") != -1; }
    void printBuffer(const char *label, const unsigned char *buf, unsigned char len);

    // Status flags
    bool wifiConnected_;
    bool mqttConnected_;
    bool canConnected_;
    bool subscriptionsInitialized_ = false;
};

#endif // CONTROLLER_H
