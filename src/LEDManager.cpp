#include "LEDManager.h"
#include <Adafruit_NeoPixel.h>

LEDManager::LEDManager(Adafruit_NeoPixel& strip) : strip_(strip) {}

void LEDManager::setup() {
  // Initialize NeoPixel
  strip_.begin();
  strip_.show(); // Initialize all pixels to 'off'
  strip_.setBrightness(64); // Set brightness (0-255)
  Serial.println("LED Manager setup complete.");
}

void LEDManager::updateStatus(bool wifiConnected, bool mqttConnected, bool canConnected) {
  if (!wifiConnected) {
    // Solid red when WiFi is not connected
    strip_.setPixelColor(0, strip_.Color(255, 0, 0)); // Red
    strip_.show();
  } else if (!mqttConnected || !canConnected) {
    // Solid yellow when MQTT or CAN is not connected
    strip_.setPixelColor(0, strip_.Color(255, 165, 0)); // Orange
    strip_.show();
  } else {
    // Solid green when everything is connected
    strip_.setPixelColor(0, strip_.Color(0, 255, 0)); // Green
    strip_.show();
  }
}