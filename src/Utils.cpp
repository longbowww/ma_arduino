#include <WiFi.h> // Ensure this header is included
#include "Utils.h"

String getMacAddressAsString() {
    uint8_t mac[6]; // Replace WL_MAC_ADDR_LENGTH with 6
    WiFi.macAddress(mac);
    char macStr[18]; // MAC address string length (17) + null terminator

    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return String(macStr);
}