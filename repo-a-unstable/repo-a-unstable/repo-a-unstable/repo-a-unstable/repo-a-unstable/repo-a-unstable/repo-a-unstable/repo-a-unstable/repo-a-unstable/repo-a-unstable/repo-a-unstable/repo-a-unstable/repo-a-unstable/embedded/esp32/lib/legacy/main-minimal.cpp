#include <Arduino.h>
#include "SIM7080.h"

SIM7080 modem;

void setup()
{
  Serial.begin(115200);
  modem.initialize();
  modem.start();
  modem.setupNetwork();
}

void loop()
{
  modem.sendCommand("AT");
  delay(30000);
}
