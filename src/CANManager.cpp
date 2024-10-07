#include "CANManager.h"
#include <Arduino.h>

CANManager::CANManager(int csPin, long speed)
  : can_(csPin), speed_(speed) {}

bool CANManager::begin() {
  if (can_.begin(MCP_ANY, speed_, MCP_16MHZ) == CAN_OK) {
    Serial.println("MCP2515 Initialized Successfully!");
    can_.setMode(MCP_NORMAL);
    return true;
  } else {
    Serial.println("Error Initializing MCP2515...");
    return false;
  }
}

bool CANManager::hasMessage() {
  return can_.checkReceive() == CAN_MSGAVAIL;
}

bool CANManager::readMessage(unsigned long& id, unsigned char& len, unsigned char* buf) {
  return can_.readMsgBuf(&id, &len, buf);
}

bool CANManager::sendMessage(unsigned long id, unsigned char len, const unsigned char* buf) {
  return can_.sendMsgBuf(id, 0, len, buf) == CAN_OK;
}
