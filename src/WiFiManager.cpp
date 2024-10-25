// WiFiManager.cpp

#include "WiFiManager.h"
#include <Arduino.h>

// Constructor implementation
WiFiManager::WiFiManager(const char* ssid, const char* password)
    : ssid_(ssid), password_(password) {
    // Additional initialization if needed
}

void WiFiManager::begin() {
    WiFi.begin(ssid_, password_);
    WiFi.setSleep(false);
    Serial.print("Connecting to WiFi");

    unsigned long startAttemptTime = millis();
    const unsigned long timeout = 20000; // 20 seconds timeout

    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startAttemptTime > timeout) {
            Serial.println("\nFailed to connect to WiFi");
            return; // Exit if timeout occurs
        }

        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected to WiFi");
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}
