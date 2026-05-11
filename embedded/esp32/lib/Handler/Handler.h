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
#include "../RGBLight/RGBLight.h"
#include "../Debug/Debug.h"

class Handler {
private:
  DeepSleepManager deepSleepManager;
  TaskHandle_t rgbTaskHandle = nullptr;
  static void rgbTask(void *pvParameters);

public:
  void setupRGB(RGBLight &rgb);
  void rgbInitialization(RGBLight &rgb);
  void disableRGB(RGBLight &rgb);
  void errorEncounteredRGB(RGBLight &rgb, SetupError error);
  void setupSPS30(SPS30 &sps30, TwoWire &wire, RGBLight &rgb);
  void setupSHT41(SHT41Sensor &sht41, TwoWire &wire, RGBLight &rgb);
  void setupWifi(WiFiManager &network, NetworkServer &server, RGBLight &rgb);
  void setupSim7080(SIM7080 &sim7080, RGBLight &rgb);
  void setupCredentialManager(CredentialManager &credential_manager,
                              NetworkServer &server, RGBLight &rgb);

  void enterDeepSleep(LEDStrip &strip, SegmentDisplay &segmentDisplay,
                      bool useSIM);

};