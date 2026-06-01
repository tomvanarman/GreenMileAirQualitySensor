#include "HelpMethod.h"

#include <DEBUG.h>
#include <DeepSleepManager.h>

time_t DeepSleepManager::getNextQuarterHourEpoch() {
  time_t now;
  time(&now);

  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  timeinfo.tm_sec = 0;

  if (timeinfo.tm_min < 15) {
    timeinfo.tm_min = 15;
  } else if (timeinfo.tm_min < 30) {
    timeinfo.tm_min = 30;
  } else if (timeinfo.tm_min < 45) {
    timeinfo.tm_min = 45;
  } else {
    timeinfo.tm_min = 0;
    timeinfo.tm_hour += 1;
  }

  return mktime(&timeinfo);
}

uint64_t DeepSleepManager::getSleepTimeToNextQuarterHourUs() {
  time_t now;
  time(&now);

  time_t nextQuarter = getNextQuarterHourEpoch();
  time_t sleepSeconds = nextQuarter - now;

  if (sleepSeconds <= 0) {
    sleepSeconds = 1;
  }

  return static_cast<uint64_t>(sleepSeconds) * 1000000ULL;
}

void DeepSleepManager::enterDeepSleep(LEDStrip &strip,
                                      SegmentDisplay &segmentDisplay,
                                      bool useSIM) {
  DEBUG_SECTION("Deep Sleep");

  time_t now;
  time(&now);

  if (now < 100000) {
    DEBUG_WARN("Current time is not valid, fallback sleep for 60 seconds");

    strip.clear();
    segmentDisplay.clearDisplay();

    if (!useSIM) {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
    }

    esp_sleep_enable_timer_wakeup(60ULL * 1000000ULL);
    delay(100);
    esp_deep_sleep_start();
    return;
  }

  struct tm nowInfo;
  localtime_r(&now, &nowInfo);

  time_t nextQuarter = getNextQuarterHourEpoch();

  struct tm nextInfo;
  localtime_r(&nextQuarter, &nextInfo);

  uint64_t sleepTimeUs = getSleepTimeToNextQuarterHourUs();

  DEBUG_INFO("Entering deep sleep until next quarter");
  DEBUG_KV("Current time", String(asctime(&nowInfo)));
  DEBUG_KV("Next wake time", String(asctime(&nextInfo)));
  DEBUG_KV("Sleep duration (seconds)", sleepTimeUs / 1000000ULL);

  strip.clear();
  segmentDisplay.clearDisplay();

  if (!useSIM) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }

  esp_sleep_enable_timer_wakeup(sleepTimeUs);
  wait(100);
  esp_deep_sleep_start();
}