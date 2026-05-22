#pragma once

#include <Wire.h>

#include "../CredentialManager/CredentialManager.h"
#include "../Debug/Debug.h"
#include "../DeepSleepManager/DeepSleepManager.h"
#include "../HelpMethod/HelpMethod.h"
#include "../LEDStrip/LEDStrip.h"
#include "../NetworkServer/NetworkServer.h"
#include "../RGBLight/RGBLight.h"
#include "../SHT41Sensor/SHT41Sensor.h"
#include "../SIM7080/SIM7080.h"
#include "../SPS30/SPS30.h"
#include "../SegmentDisplay/SegmentDisplay.h"
#include "../WifiManager/WifiManager.h"

// clang-format off
class Handler {
 private:
  DeepSleepManager deepSleepManager;
  TaskHandle_t rgbTaskHandle = nullptr;
  TaskHandle_t errorTaskHandle = nullptr;
  static void rgbTask(void* pvParameters);
  static void errorTask(void* pvParameters);

  // Struct to pass RGB and error to error task
  struct ErrorTaskParams {
    RGBLight* rgb;
    SetupError error;
  };

 public:
  void setupRGB(RGBLight& rgb);  // NOLINT(runtime/references)
  void rgbInitialization(RGBLight& rgb);  // NOLINT(runtime/references)
  void disableRGB(RGBLight& rgb);  // NOLINT(runtime/references)
  void errorEncounteredRGB(RGBLight& rgb,  // NOLINT(runtime/references)
                           SetupError error);
  void startErrorTask(RGBLight& rgb,  // NOLINT(runtime/references)
                      SetupError error);
  bool setupSPS30(SPS30& sps30,  // NOLINT(runtime/references)
                  TwoWire& wire,  // NOLINT(runtime/references)
                  RGBLight& rgb);  // NOLINT(runtime/references)
  bool setupSHT41(SHT41Sensor& sht41,  // NOLINT(runtime/references)
                  TwoWire& wire,  // NOLINT(runtime/references)
                  RGBLight& rgb);  // NOLINT(runtime/references)
  void setupWifi(WiFiManager& network,  // NOLINT(runtime/references)
                 NetworkServer& server,  // NOLINT(runtime/references)
                 RGBLight& rgb);  // NOLINT(runtime/references)
  void setupSim7080(SIM7080& sim7080,  // NOLINT(runtime/references)
                    RGBLight& rgb);  // NOLINT(runtime/references)
  void setupCredentialManager(
      CredentialManager& credential_manager,  // NOLINT(runtime/references)
      NetworkServer& server,  // NOLINT(runtime/references)
      RGBLight& rgb);  // NOLINT(runtime/references)

  void enterDeepSleep(
      LEDStrip& strip,  // NOLINT(runtime/references)
      SegmentDisplay& segmentDisplay,  // NOLINT(runtime/references)
      SPS30& sps30,  // NOLINT(runtime/references)
      TwoWire& sensorWire,  // NOLINT(runtime/references)
      SIM7080& sim7080,  // NOLINT(runtime/references)
      int sensorSdaPin,
      int sensorSclPin,
      bool useSIM);
};
// clang-format on
