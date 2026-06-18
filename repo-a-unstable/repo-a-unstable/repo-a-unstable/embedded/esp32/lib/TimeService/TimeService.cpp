#include "TimeService.h"

#include <sys/time.h>
#include <time.h>

#include "DEBUG.h"

void TimeService::configureLocalTimezone() {
    setenv("TZ", kLocalTimeZone, 1);
    tzset();
}

void TimeService::initializeNtpTime() {
    DEBUG_SECTION("NTP Sync");
    configureLocalTimezone();
    configTzTime(kLocalTimeZone, kNtpServer1, kNtpServer2);
    DEBUG_INFO("Waiting for NTP time sync...");

    struct tm timeinfo;
    for (uint8_t attempt = 0; attempt < 3; attempt++) {
        if (getLocalTime(&timeinfo, 1000)) {
            DEBUG_OK("Time synchronized");
            char timeBuffer[32];
            strftime(timeBuffer, sizeof(timeBuffer), "%a %b %d %H:%M:%S %Y",
                     &timeinfo);
            DEBUG_KV("Current time", String(timeBuffer));
            return;
        }
        DEBUG_WARN("Failed to obtain time, retrying...");
        yield();
    }

    DEBUG_WARN("NTP time sync skipped after quick retries");
}

void TimeService::syncFromSIM(SIM7080& sim7080) {
    DEBUG_SECTION("SIM Time Sync");

    uint64_t timestampMs = sim7080.getCurrentTimestampMs();
    if (timestampMs == 0) {
        DEBUG_WARN("Could not sync time from SIM7080");
        return;
    }

    timeval tv;
    tv.tv_sec = timestampMs / 1000;
    tv.tv_usec = (timestampMs % 1000) * 1000;

    if (settimeofday(&tv, nullptr) != 0) {
        DEBUG_WARN("Failed to set system time from SIM7080");
        return;
    }

    configureLocalTimezone();
    DEBUG_OK("System time synchronized from SIM7080");
}

uint64_t TimeService::currentTimestampMs() const {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return static_cast<uint64_t>(tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
}
