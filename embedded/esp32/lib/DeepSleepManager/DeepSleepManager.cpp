#include <DEBUG.h>
#include <DeepSleepManager.h>
#include <esp_bt.h>
#include <esp_wifi.h>

#include "HelpMethod.h"

time_t DeepSleepManager::getNextWakeEpoch(WakeSchedule schedule) {
    time_t now;
    time(&now);

    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    const int intervalMinutes =
        schedule == WakeSchedule::EveryQuarterHour ? 15 : 5;
    const int minutesPastInterval = timeinfo.tm_min % intervalMinutes;

    timeinfo.tm_sec = 0;
    timeinfo.tm_min += intervalMinutes - minutesPastInterval;

    return mktime(&timeinfo);
}

uint64_t DeepSleepManager::getSleepTimeToNextWakeUs(WakeSchedule schedule) {
    time_t now;
    time(&now);

    time_t nextWake = getNextWakeEpoch(schedule);
    time_t sleepSeconds = nextWake - now;

    if (sleepSeconds <= 0) {
        sleepSeconds = 1;
    }

    return static_cast<uint64_t>(sleepSeconds) * 1000000ULL;
}

void DeepSleepManager::preparePeripheralsForDeepSleep(
    SPS30& sps30, TwoWire& sensorWire, SIM7080& sim7080, int sensorSdaPin,
    int sensorSclPin, bool useSIM) {
    DEBUG_SECTION("Deep Sleep Peripheral Shutdown");

    if (sps30.isInitialized()) {
        if (sps30.sleep()) {
            DEBUG_OK("SPS30 entered sensor sleep mode");
        } else {
            DEBUG_WARN("SPS30 sleep command failed");
        }
    }
    sps30.resetState();

    sensorWire.end();

    pinMode(sensorSdaPin, INPUT);
    pinMode(sensorSclPin, INPUT);

    if (!useSIM) {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        esp_wifi_stop();
    }

    btStop();
    esp_bt_controller_disable();

    sim7080.shutdownForDeepSleep();
}

void DeepSleepManager::enterDeepSleep(SPS30& sps30, TwoWire& sensorWire,
                                      SIM7080& sim7080, int sensorSdaPin,
                                      int sensorSclPin, bool useSIM) {
    DEBUG_SECTION("Deep Sleep");

    time_t now;
    time(&now);

    if (now < 100000) {
        DEBUG_WARN("Current time is not valid, fallback sleep for 60 seconds");

        preparePeripheralsForDeepSleep(sps30, sensorWire, sim7080, sensorSdaPin,
                                       sensorSclPin, useSIM);

        esp_sleep_enable_timer_wakeup(60ULL * 1000000ULL);
        DEBUG_INFO("ESP32 entering deep sleep now");
        wait(100);
        esp_deep_sleep_start();
        return;
    }

    struct tm nowInfo;
    localtime_r(&now, &nowInfo);

    const WakeSchedule wakeSchedule = DEFAULT_WAKE_SCHEDULE;
    time_t nextWake = getNextWakeEpoch(wakeSchedule);

    struct tm nextInfo;
    localtime_r(&nextWake, &nextInfo);

    uint64_t sleepTimeUs = getSleepTimeToNextWakeUs(wakeSchedule);
    char currentTimeText[26];
    char nextWakeTimeText[26];
    asctime_r(&nowInfo, currentTimeText);
    asctime_r(&nextInfo, nextWakeTimeText);

    DEBUG_INFO(wakeSchedule == WakeSchedule::EveryQuarterHour
                   ? "Entering deep sleep until next quarter hour"
                   : "Entering deep sleep until next 5-minute mark");
    DEBUG_KV("Current time", String(currentTimeText));
    DEBUG_KV("Next wake time", String(nextWakeTimeText));
    DEBUG_KV("Sleep duration (seconds)", sleepTimeUs / 1000000ULL);

    preparePeripheralsForDeepSleep(sps30, sensorWire, sim7080, sensorSdaPin,
                                   sensorSclPin, useSIM);

    esp_sleep_enable_timer_wakeup(sleepTimeUs);
    DEBUG_INFO("ESP32 entering deep sleep now");
    wait(100);
    esp_deep_sleep_start();
}
