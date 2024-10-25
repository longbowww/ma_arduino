#include "Controller.h"
#include "LEDManager.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// LED Configuration
#define LED_PIN 48 // GPIO48 where the NeoPixel data line is connected
#define NUM_LEDS 1

// Other configurations...
#define WIFI_SSID "UniFi"
#define WIFI_PASSWORD "Eikki1798!"
#define MQTT_SERVER "192.168.1.198"
#define MQTT_PORT 1883
#define CAN_INT_PIN 3
#define CAN_CS_PIN 10
#define CAN_SPEED CAN_500KBPS
#define RECONNECT_DELAY 5000

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

LEDManager ledManager(strip);

Controller::Controller(int vehicleNum)
    : wifiManager_(WIFI_SSID, WIFI_PASSWORD),
      mqttManager_(MQTT_SERVER, MQTT_PORT),
      canManager_(CAN_CS_PIN, CAN_SPEED),
      vehicle_(vehicleNum),
      wifiConnected_(false),
      mqttConnected_(false),
      canConnected_(false) {}

void Controller::setup()
{
  // Initialize LED Manager
  Serial.println("Setting up LED Manager...");
  ledManager.setup();

  // Initialize Serial
  Serial.begin(115200);
  delay(1000); // Wait for Serial to initialize
  Serial.println("Serial initialized.");

  // Initialize WiFi
  Serial.println("Initializing WiFi...");
  wifiManager_.begin();

  // Initialize MQTT
  Serial.println("Initializing MQTT...");
  mqttManager_.begin();

  // Initialize CAN
  Serial.println("Initializing CAN...");
  if (!canManager_.begin())
  {
    Serial.println("CAN Initialization Failed. Continuing to retry.");
    canConnected_ = false;
    // Continue running to attempt reconnection later
  }
  else
  {
    Serial.println("CAN Initialized successfully.");
    canConnected_ = true;
  }
}

void Controller::loop()
{
  // Update the status LED based on connection statuses
  ledManager.updateStatus(wifiConnected_, mqttConnected_, canConnected_);

  // Update WiFi connection status
  wifiConnected_ = wifiManager_.isConnected();
  // Ensure MQTT is connected
  if (!mqttManager_.isConnected())
  {
    mqttManager_.reconnect();
    mqttConnected_ = mqttManager_.isConnected();
  }
  else
  {
    mqttConnected_ = true;
  }

  mqttManager_.loop();

  // Handle incoming CAN messages
  if (canConnected_)
  {
    processCANMessages();
  }
  else
  {
    if (canManager_.begin())
    {
      pinMode(3, INPUT);
      canConnected_ = true;
    }
    else
    {
      canConnected_ = false;
    }
  }

  delay(100);
  // Optional: Add other periodic tasks here
}

void Controller::processCANMessages()
{
  /* FOR DEBUG !!!ONLY!!
  // Serial.println("Processing CAN messages...");
  if (canManager_.hasMessage()) {
    unsigned long int canId;
    unsigned char len = 0;
    unsigned char buf[8];
    char msgString[256];

    canManager_.readMessage(canId, len, buf);


    if((canId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (canId & 0x1FFFFFFF), len);
    else
      sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", canId, len);

    Serial.print(msgString);

    if((canId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } else {
      for(byte i = 0; i<len; i++){
        sprintf(msgString, " 0x%.2X", buf[i]);
        Serial.print(msgString);
      }
    }

    Serial.println(msgString);
  } */

  if (canManager_.hasMessage())
  {
    unsigned long int canId;
    unsigned char len = 0;
    unsigned char buf[8];

    if (canManager_.readMessage(canId, len, buf))
    {
      Serial.print("Received CAN ID: ");
      Serial.println(canId, HEX);

      if (canId == 0x69 && len >= 4)
      { // Ensure the CAN ID and enough data bytes
        // Decode speed and distance from the CAN message
        uint16_t rawSpeed = (buf[0] << 8) | buf[1];
        uint16_t rawDistance = (buf[2] << 8) | buf[3];

        // Convert to fixed-point values
        vehicle_.speed = rawSpeed / 100.0;       // Fixed-point to float
        vehicle_.distance = rawDistance / 100.0; // Fixed-point to float

        Serial.print("Vehicle ");
        Serial.print(vehicle_.number);
        Serial.print(" - Speed: ");
        Serial.print(vehicle_.speed);
        Serial.print(" m/s, Distance: ");
        Serial.print(vehicle_.distance);
        Serial.println(" m");

        Serial.println("Publishing vehicle data...");
        publishVehicleData();
      }
      else
      {
        Serial.println("Received CAN message with unexpected ID or insufficient length.");
      }
    }
    else
    {
      Serial.println("tried but failed :(");
    }
  }
}

void Controller::publishVehicleData() {
  // Construct MQTT topics
  String speedTopic = "/vehicles/" + String(vehicle_.number) + "/speed";
  String distanceTopic = "/vehicles/" + String(vehicle_.number) + "/distance";

  // Convert data to string
  String speedStr = String(vehicle_.speed);
  String distanceStr = String(vehicle_.distance);

  // Publish speed
  Serial.print("Publishing speed to topic: ");
  Serial.println(speedTopic);
  if (!mqttManager_.publish(speedTopic, speedStr )) {
    Serial.println("Failed to publish speed");
    // mqttConnected_ = false; // Commented out as immediate disconnection assumption is not always valid
  }

  // Publish distance
  Serial.print("Publishing distance to topic: ");
  Serial.println(distanceTopic);
  if (!mqttManager_.publish(distanceTopic, distanceStr))
  {
    Serial.println("Failed to publish distance");
    // mqttConnected_ = false; // Commented out as immediate disconnection assumption is not always valid
  }
}
