#include "MQTTManager.h"
#include "Utils.h" // Include the utility header
#include <Arduino.h>

#define RECONNECT_DELAY 5000

MQTTManager::MQTTManager(const char* server, uint16_t port)
    : mqttServer_(server), mqttPort_(port), client_(espClient_) {}

void MQTTManager::begin() {
    client_.setServer(mqttServer_, mqttPort_);
    client_.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->callback(topic, payload, length);
    });
}

void MQTTManager::reconnect() {
    while (!client_.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Generate a unique client ID using the MAC address
        String clientId = "ESPClient-" + getMacAddressAsString();
        if (client_.connect(clientId.c_str())) {
            Serial.println(" connected to MQTT");
            // Subscribe to necessary topics if needed
            // client_.subscribe("your/subscribe/topic");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client_.state());
            Serial.println(" try again in 5 seconds");
            delay(RECONNECT_DELAY);
        }
    }
}

void MQTTManager::loop() {
    client_.loop();
}

bool MQTTManager::publish(const String& topic, const String& message) {
    return client_.publish(topic.c_str(), message.c_str());
}

bool MQTTManager::isConnected() const {
    return client_.connected();
}

void MQTTManager::callback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0'; // Null-terminate the payload
    String message = String((char*)payload);
    Serial.print("MQTT Message Received - Topic: ");
    Serial.print(topic);
    Serial.print(", Message: ");
    Serial.println(message);

    // Handle incoming messages as needed
    // Example:
    /*
    if (strcmp(topic, "control/motors") == 0) {
        int speed = message.toInt();
        // Handle speed control
    }
    */
}
