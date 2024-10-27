// MQTTManager.h
#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <PubSubClient.h>
#include <WiFi.h>
#include <functional>
#include <vector>

class MQTTManager {
public:
    MQTTManager(const char* server, uint16_t port);
    void begin();
    bool reconnect();
    void loop();
    bool publish(const String& topic, const String& message);
    bool isConnected();
    bool subscribe(const String& topic);
    void setOnMessageCallback(std::function<void(const String&, const String&)> callback);

private:
    void callback(char* topic, byte* payload, unsigned int length);
    const char* mqttServer_;
    uint16_t mqttPort_;
    WiFiClient espClient_;
    PubSubClient client_;
    std::vector<String> subscribedTopics_;
    std::function<void(const String&, const String&)> messageCallback_;  // Renamed for clarity
};

#endif // MQTT_MANAGER_H