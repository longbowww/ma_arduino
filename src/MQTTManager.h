#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <PubSubClient.h>
#include <WiFi.h>
#include <functional>

class MQTTManager {
public:
    MQTTManager(const char* server, uint16_t port);
    void begin();
    void reconnect();
    void loop();
    bool publish(const String& topic, const String& message);
    bool isConnected();

private:
    void callback(char* topic, byte* payload, unsigned int length);
    const char* mqttServer_;
    uint16_t mqttPort_;
    WiFiClient espClient_;
    PubSubClient client_;
};

#endif // MQTT_MANAGER_H
