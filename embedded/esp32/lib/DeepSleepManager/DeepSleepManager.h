#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <esp_sleep.h>
#include <time.h>

#include "../LEDStrip/LEDStrip.h"
#include "../SIM7080/SIM7080.h"
#include "../SPS30/SPS30.h"
#include "../SegmentDisplay/SegmentDisplay.h"

class DeepSleepManager {
public:
  void enterDeepSleep(LEDStrip &strip, SegmentDisplay &segmentDisplay,
                      SPS30 &sps30, TwoWire &sensorWire, SIM7080 &sim7080,
                      int sensorSdaPin, int sensorSclPin, bool useSIM);

private:
  void preparePeripheralsForDeepSleep(LEDStrip &strip,
                                      SegmentDisplay &segmentDisplay,
                                      SPS30 &sps30, TwoWire &sensorWire,
                                      SIM7080 &sim7080, int sensorSdaPin,
                                      int sensorSclPin, bool useSIM);
  time_t getNextQuarterHourEpoch();
  uint64_t getSleepTimeToNextQuarterHourUs();
};
