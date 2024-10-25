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
    static unsigned long lastAttemptTime = 0;
    const unsigned long retryInterval = 5000; // 5 seconds

    if (!client_.connected() && millis() - lastAttemptTime > retryInterval) {
        lastAttemptTime = millis();
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESPClient-" + getMacAddressAsString();

        if (client_.connect(clientId.c_str())) {
            Serial.println(" connected to MQTT");
            // Subscribe to necessary topics if needed
            // client_.subscribe("your/subscribe/topic");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client_.state());
            Serial.println(" retrying...");
        }
    }
}

void MQTTManager::loop() {
    client_.loop();
}

bool MQTTManager::publish(const String& topic, const String& message) {
    // always send retained: true, so new clients will receive the last measurement
    return client_.publish(topic.c_str(), message.c_str(), true);
}

bool MQTTManager::isConnected() {
    return client_.connected();
}

void MQTTManager::callback(char* topic, byte* payload, unsigned int length) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0'; // Null-terminate the copied payload
    String msgString = String(message);

    Serial.print("MQTT Message Received - Topic: ");
    Serial.print(topic);
    Serial.print(", Message: ");
    Serial.println(msgString);

    // Handle incoming messages as needed
    // Example:
    /*
    if (strcmp(topic, "control/motors") == 0) {
        int speed = msgString.toInt();
        // Handle speed control
    }
    */
}
