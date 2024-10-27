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
    delay(1000);
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
    }
    else
    {
        Serial.println("CAN Initialized successfully.");
        canConnected_ = true;
    }
}

void Controller::setupSubscriptions()
{
    Serial.print("Vehicle number: ");
    Serial.println(vehicle_.number);

    if (vehicle_.number == 1)
    {
        // Lead vehicle (1) subscribes to command topic for speed control
        String commandTopic = formatTopic(TOPIC_PATTERN_COMMAND, vehicle_.number);
        Serial.printf("Lead vehicle attempting to subscribe to: %s\n", commandTopic.c_str());
        if (mqttManager_.subscribe(commandTopic))
        {
            Serial.printf("Successfully subscribed to: %s\n", commandTopic.c_str());
        }
        else
        {
            Serial.printf("Failed to subscribe to: %s\n", commandTopic.c_str());
        }
    }
    else
    {
        // Follower vehicles subscribe to data topics of all vehicles ahead of them
        Serial.printf("Follower vehicle %d setting up subscriptions...\n", vehicle_.number);

        for (int i = 1; i < vehicle_.number; ++i)
        {
            String dataTopic = formatTopic(TOPIC_PATTERN_DATA, i);
            Serial.printf("Attempting to subscribe to: %s\n", dataTopic.c_str());
            if (mqttManager_.subscribe(dataTopic))
            {
                Serial.printf("Successfully subscribed to: %s\n", dataTopic.c_str());
            }
            else
            {
                Serial.printf("Failed to subscribe to: %s\n", dataTopic.c_str());
            }
        }
    }

    // Set callback for all incoming messages
    mqttManager_.setOnMessageCallback([this](const String &topic, const String &payload)
                                      { this->onReceivedMessage(topic, payload); });

    Serial.println("Subscription setup completed");
}

String Controller::formatTopic(const char *pattern, int vehicleNumber) const
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), pattern, vehicleNumber);
    return String(buffer);
}

void Controller::loop()
{
    // Update the status LED based on connection statuses
    ledManager.updateStatus(wifiConnected_, mqttConnected_, canConnected_);

    // Update WiFi connection status
    wifiConnected_ = wifiManager_.isConnected();

    // Handle MQTT connection and subscriptions
    if (!mqttManager_.isConnected())
    {
        mqttConnected_ = false;
        subscriptionsInitialized_ = false; // Reset subscription flag when disconnected
        if (mqttManager_.reconnect())
        {
            mqttConnected_ = true;
            Serial.println("MQTT reconnected successfully");
        }
    }
    else
    {
        mqttConnected_ = true;

        // Set up subscriptions only once after connection is established
        if (!subscriptionsInitialized_ && mqttConnected_)
        {
            Serial.println("Setting up MQTT subscriptions...");
            setupSubscriptions();
            subscriptionsInitialized_ = true;
            Serial.println("MQTT subscriptions completed");
        }
    }

    mqttManager_.loop();

    // Handle CAN connection
    if (!canConnected_)
    {
        if (canManager_.begin())
        {
            pinMode(3, INPUT);
            canConnected_ = true;
            Serial.println("CAN connected successfully");
        }
    }

    // Process CAN messages if connected
    if (canConnected_)
    {
        processCANMessages();
    }

    delay(100);
}

void Controller::processCANMessages()
{
    if (canManager_.hasMessage())
    {
        unsigned long int canId;
        unsigned char len = 0;
        unsigned char buf[8]; // Buffer to store the CAN message data

        if (canManager_.readMessage(canId, len, buf))
        {
            // Only process if the CAN ID is expected
            if (canId == 0x69 && len >= 4)
            {
                publishVehicleData(buf, len);
            }
            else
            {
                Serial.println("Received CAN message with unexpected ID or insufficient length.");
            }
        }
    }
}

void Controller::publishVehicleData(const unsigned char *buf, unsigned char len)
{
    String dataTopic = "/vehicle/" + String(vehicle_.number) + "/data";

    // Convert the raw buffer to a hex string
    String hexPayload;
    hexPayload.reserve(len * 2); // Preallocate space for efficiency

    for (int i = 0; i < len; ++i)
    {
        if (buf[i] < 0x10)
        {
            hexPayload += '0'; // Add leading zero for single-digit hex values
        }
        hexPayload += String(buf[i], HEX);
    }

    // Publish the hex string
    if (!mqttManager_.publish(dataTopic, hexPayload.c_str()))
    {
        Serial.println("Failed to publish raw CAN data");
    }
}

void Controller::onReceivedMessage(const String &topic, const String &payload)
{
    int vehicleId = extractVehicleId(topic);

    Serial.printf("Received message on topic: %s with payload: %s\n", topic.c_str(), payload.c_str());

    // Handle command messages
    if (isCommandTopic(topic))
    {
        if (vehicleId == vehicle_.number)
        {
            Serial.println("Processing command message");
            uint8_t speed = payload.toInt();
            Serial.printf("Parsed speed: %d\n", speed);
            handleSpeedCommand(speed);
        }
        else
        {
            Serial.printf("Ignoring command for vehicle %d (we are %d)\n",
                          vehicleId, vehicle_.number);
        }
    }
    // Handle data messages
    else if (isDataTopic(topic) && vehicleId != vehicle_.number)
    {
        handleVehicleRelay(vehicleId, payload);
    }
    else
    {
        Serial.printf("Message type not recognized or not for us (topic: %s)\n", topic.c_str());
    }
}

// In Controller.cpp
void Controller::handleSpeedCommand(uint8_t speed) {
    unsigned char buf[1];
    // assumption is that we only send 0->255 manually as speed for leader
    buf[0] = static_cast<unsigned char>(speed);

    printBuffer("Sending speed command:", buf, 2);

    // Send with ID 0x66 (CAN_ID_CONTROL)
    unsigned char tempBuf[8]; // Temporary buffer for CAN manager
    memcpy(tempBuf, buf, 2);

    if (!canManager_.sendMessage(CAN_ID_CONTROL, 1, tempBuf)) {
        Serial.println("Failed to send speed command via CAN");
    }
}

void Controller::printBuffer(const char *label, const unsigned char *buf, unsigned char len) {
    Serial.print(label);
    for (unsigned char i = 0; i < len; i++)
    {
        Serial.printf(" %02X", buf[i]);
    }
    Serial.println();
}

void Controller::handleVehicleRelay(int sourceVehicleId, const String &payload)
{
    Serial.printf("Relaying data for vehicle %d: %s\n", sourceVehicleId, payload.c_str());

    const char *rawData = payload.c_str();
    unsigned char len = min(payload.length(), (unsigned int)8);

    unsigned char buf[8];
    memcpy(buf, rawData, len);

    sendCANMessage(CAN_ID_CONTROL, buf, len);
}

bool Controller::sendCANMessage(unsigned long canId, const unsigned char *data, unsigned char length)
{
    if (!canManager_.sendMessage(canId, length, data))
    {
        Serial.println("Failed to send CAN message");
        return false;
    }
    return true;
}

int Controller::extractVehicleId(const String &topic) const
{
    // Find position after "/vehicle/" and before next "/"
    int startPos = topic.indexOf("/vehicle/") + 9;
    int endPos = topic.indexOf("/", startPos);
    if (startPos == -1 || endPos == -1)
        return -1;

    return topic.substring(startPos, endPos).toInt();
}
