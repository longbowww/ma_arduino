#include "Controller.h"
#include <Arduino.h>

// Configuration Constants
#define WIFI_SSID "UniFi"
#define WIFI_PASSWORD "Eikki1798!"
#define MQTT_SERVER "192.168.1.198"
#define MQTT_PORT 1883
#define CAN_CS_PIN 10
#define CAN_SPEED CAN_500KBPS
#define RECONNECT_DELAY 5000

Controller::Controller(int vehicleNum)
  : wifiManager_(WIFI_SSID, WIFI_PASSWORD),
    mqttManager_(MQTT_SERVER, MQTT_PORT),
    canManager_(CAN_CS_PIN, CAN_SPEED),
    vehicle_(vehicleNum) {}

void Controller::setup() {
  // Initialize Serial
  Serial.begin(115200);
  delay(1000); // Wait for Serial to initialize

  // Initialize WiFi
  wifiManager_.begin();

  // Initialize MQTT
  mqttManager_.begin();

  // Initialize CAN
  if (!canManager_.begin()) {
    Serial.println("CAN Initialization Failed. Halting.");
    while (1); // Halt if CAN initialization fails
  }
}

void Controller::loop() {
  // Ensure MQTT is connected
  if (!mqttManager_.isConnected()) {
    mqttManager_.reconnect();
  }
  mqttManager_.loop();

  // Handle incoming CAN messages
  processCANMessages();

  // Optional: Add other periodic tasks here
}

void Controller::processCANMessages() {
  if (canManager_.hasMessage()) {
    unsigned long canId = 0;
    unsigned char len = 0;
    unsigned char buf[8];

    if (canManager_.readMessage(canId, len, buf)) {
      Serial.print("Received CAN ID: ");
      Serial.println(canId, HEX);

      // Determine vehicle number from CAN ID
      int msgVehicleNumber = canId - 0x100;
      if (msgVehicleNumber == vehicle_.number) {
        if (len >= 4) { // Ensure enough data bytes
          // Parse speed and distance
          vehicle_.speed = (buf[0] << 8) | buf[1];
          vehicle_.distance = (buf[2] << 8) | buf[3];

          Serial.print("Vehicle ");
          Serial.print(vehicle_.number);
          Serial.print(" - Speed: ");
          Serial.print(vehicle_.speed);
          Serial.print(" km/h, Distance: ");
          Serial.print(vehicle_.distance);
          Serial.println(" m");

          // Publish to MQTT
          publishVehicleData();
        } else {
          Serial.println("Received CAN message with insufficient data bytes.");
        }
      } else {
        Serial.print("Message for vehicle ");
        Serial.println(msgVehicleNumber);
      }
    }
  }
}

void Controller::publishVehicleData() {
  // Construct MQTT topics
  String speedTopic = "/v" + String(vehicle_.number) + "/speed";
  String distanceTopic = "/v" + String(vehicle_.number) + "/distance";

  // Convert data to string
  String speedStr = String(vehicle_.speed);
  String distanceStr = String(vehicle_.distance);

  // Publish speed
  if (mqttManager_.publish(speedTopic, speedStr)) {
    Serial.println("Speed published successfully");
  } else {
    Serial.println("Failed to publish speed");
  }

  // Publish distance
  if (mqttManager_.publish(distanceTopic, distanceStr)) {
    Serial.println("Distance published successfully");
  } else {
    Serial.println("Failed to publish distance");
  }
}
