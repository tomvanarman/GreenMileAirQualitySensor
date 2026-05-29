#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <esp_sleep.h>
#include <time.h>

#include "../LEDStrip/LEDStrip.h"
#include "../SegmentDisplay/SegmentDisplay.h"

class DeepSleepManager {
public:
  void enterDeepSleep(LEDStrip &strip, SegmentDisplay &segmentDisplay,
                      bool useSIM);

private:
  time_t getNextQuarterHourEpoch();
  uint64_t getSleepTimeToNextQuarterHourUs();
};