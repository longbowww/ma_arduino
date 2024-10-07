#include "Controller.h"

const int VEHICLE_NUMBER = 1; // Set your vehicle number here (1 -> n)
Controller controller(VEHICLE_NUMBER);

void setup() {
  controller.setup();
}

void loop() {
  controller.loop();
}
