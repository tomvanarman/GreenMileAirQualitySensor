#pragma once

#include <Wire.h>

#include "../CredentialManager/CredentialManager.h"
#include "../DeepSleepManager/DeepSleepManager.h"
#include "../HelpMethod/HelpMethod.h"
#include "../LEDStrip/LEDStrip.h"
#include "../NetworkServer/NetworkServer.h"
#include "../SHT41Sensor/SHT41Sensor.h"
#include "../SIM7080/SIM7080.h"
#include "../SPS30/SPS30.h"
#include "../SegmentDisplay/SegmentDisplay.h"
#include "../WifiManager/WifiManager.h"

class Handler {
private:
  DeepSleepManager deepSleepManager;

public:
  void setupSPS30(SPS30 &sps30, TwoWire &wire, LEDStrip &strip);
  void setupSHT41(SHT41Sensor &sht41, TwoWire &wire, LEDStrip &strip);
  void setupWifi(WiFiManager &network, NetworkServer &server, LEDStrip &strip,
                 SegmentDisplay &segmentDisplay);
  void setupSim7080(SIM7080 &sim7080, LEDStrip &strip);
  void setupCredentialManager(CredentialManager &credential_manager,
                              NetworkServer &server, LEDStrip &strip,
                              SegmentDisplay &segmentDisplay);

  void enterDeepSleep(LEDStrip &strip, SegmentDisplay &segmentDisplay,
                      bool useSIM);
};