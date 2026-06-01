#include <DEBUG.h>
#include <DeepSleepManager.h>
#include <esp_bt.h>
#include <esp_wifi.h>

#include "HelpMethod.h"

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

    time_t nextQuarter = getNextQuarterHourEpoch();

    struct tm nextInfo;
    localtime_r(&nextQuarter, &nextInfo);

    uint64_t sleepTimeUs = getSleepTimeToNextQuarterHourUs();
    char currentTimeText[26];
    char nextWakeTimeText[26];
    asctime_r(&nowInfo, currentTimeText);
    asctime_r(&nextInfo, nextWakeTimeText);

    DEBUG_INFO("Entering deep sleep until next quarter");
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
