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
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}
