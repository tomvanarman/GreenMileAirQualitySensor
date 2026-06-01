#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_sleep.h>
#include <time.h>

#include "../SIM7080/SIM7080.h"
#include "../SPS30/SPS30.h"

// clang-format off
class DeepSleepManager {
 public:
    void enterDeepSleep(SPS30& sps30,         // NOLINT(runtime/references)
                        TwoWire& sensorWire,  // NOLINT(runtime/references)
                        SIM7080& sim7080,     // NOLINT(runtime/references)
                        int sensorSdaPin, int sensorSclPin, bool useSIM);

 private:
    void preparePeripheralsForDeepSleep(
        SPS30& sps30,         // NOLINT(runtime/references)
        TwoWire& sensorWire,  // NOLINT(runtime/references)
        SIM7080& sim7080,     // NOLINT(runtime/references)
        int sensorSdaPin, int sensorSclPin, bool useSIM);
    time_t getNextQuarterHourEpoch();
    uint64_t getSleepTimeToNextQuarterHourUs();
};
// clang-format on
