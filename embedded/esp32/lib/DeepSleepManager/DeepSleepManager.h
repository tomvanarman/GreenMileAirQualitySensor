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
    enum class WakeSchedule {
        EveryFiveMinutes,
        EveryQuarterHour,
    };

    void enterDeepSleep(SPS30& sps30,         // NOLINT(runtime/references)
                        TwoWire& sensorWire,  // NOLINT(runtime/references)
                        SIM7080& sim7080,     // NOLINT(runtime/references)
                        int sensorSdaPin, int sensorSclPin, bool useSIM);

 private:
    static constexpr WakeSchedule DEFAULT_WAKE_SCHEDULE =
        WakeSchedule::EveryFiveMinutes;

    void preparePeripheralsForDeepSleep(
        SPS30& sps30,         // NOLINT(runtime/references)
        TwoWire& sensorWire,  // NOLINT(runtime/references)
        SIM7080& sim7080,     // NOLINT(runtime/references)
        int sensorSdaPin, int sensorSclPin, bool useSIM);
    time_t getNextWakeEpoch(WakeSchedule schedule);
    uint64_t getSleepTimeToNextWakeUs(WakeSchedule schedule);
};
// clang-format on
