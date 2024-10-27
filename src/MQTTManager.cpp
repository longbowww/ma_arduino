#include "MQTTManager.h"
#include "Utils.h"
#include <Arduino.h>

MQTTManager::MQTTManager(const char* server, uint16_t port)
    : mqttServer_(server), mqttPort_(port), client_(espClient_) {}

void MQTTManager::begin() {
    client_.setServer(mqttServer_, mqttPort_);
    client_.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->callback(topic, payload, length);
    });
}

bool MQTTManager::reconnect() {
    static unsigned long lastAttemptTime = 0;
    const unsigned long retryInterval = 500;

    if (!client_.connected() && millis() - lastAttemptTime > retryInterval) {
        lastAttemptTime = millis();
        Serial.print("Attempting MQTT connection...");
        
        String clientId = "ESPClient-" + getMacAddressAsString();
        if (client_.connect(clientId.c_str())) {
            Serial.println(" connected to MQTT");
            
            // Re-subscribe to all topics
            for (const String& topic : subscribedTopics_) {
                Serial.printf("Resubscribing to topic: %s\n", topic.c_str());
                if (!client_.subscribe(topic.c_str())) {
                    Serial.printf("Failed to resubscribe to: %s\n", topic.c_str());
                }
            }
            return true;
        } else {
            Serial.print("failed, rc=");
            Serial.print(client_.state());
            Serial.println(" retrying...");
            return false;
        }
    }
    return client_.connected();
}

bool MQTTManager::subscribe(const String& topic) {
    // Check if we're already subscribed to this topic
    bool alreadySubscribed = false;
    for (const String& existingTopic : subscribedTopics_) {
        if (existingTopic == topic) {
            alreadySubscribed = true;
            break;
        }
    }

    // If not already subscribed, add to our list
    if (!alreadySubscribed) {
        subscribedTopics_.push_back(topic);
        Serial.printf("Added topic to subscription list: %s\n", topic.c_str());
    }

    // Always attempt to subscribe if connected
    if (client_.connected()) {
        bool success = client_.subscribe(topic.c_str());
        Serial.printf("Subscription attempt to %s: %s\n", 
                     topic.c_str(), 
                     success ? "successful" : "failed");
        return success;
    }
    
    return false;
}

void MQTTManager::loop() {
    client_.loop();
}

bool MQTTManager::publish(const String& topic, const String& message) {
    if (!client_.connected()) {
        Serial.println("Cannot publish: MQTT not connected");
        return false;
    }
    return client_.publish(topic.c_str(), message.c_str(), true);
}

bool MQTTManager::isConnected() {
    return client_.connected();
}

void MQTTManager::callback(char* topic, byte* payload, unsigned int length) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    
    String topicStr = String(topic);
    String messageStr = String(message);
    
    Serial.printf("MQTT Message - Topic: %s, Payload: %s\n", 
                 topicStr.c_str(), 
                 messageStr.c_str());

    // Call the callback if it exists, regardless of topic
    if (messageCallback_) {
        messageCallback_(topicStr, messageStr);
    } else {
        Serial.println("Warning: No message callback set");
    }
}

void MQTTManager::setOnMessageCallback(std::function<void(const String&, const String&)> callback) {
    messageCallback_ = callback;
    Serial.println("Message callback set");
}