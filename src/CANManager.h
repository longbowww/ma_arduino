#ifndef CAN_MANAGER_H
#define CAN_MANAGER_H

#include <mcp_can.h>
#include <SPI.h>

class CANManager {
public:
  CANManager(int csPin, long speed);
  bool begin();
  bool hasMessage();
  bool readMessage(unsigned long& id, unsigned char& len, unsigned char* buf);
  bool sendMessage(unsigned long id, unsigned char len, unsigned char* buf);

private:
  MCP_CAN can_;
  long speed_;
};

#endif // CAN_MANAGER_H
