#pragma once // Only include this file once
#include <Arduino.h>
#include "DEBUG.h"

class SIM7080
{
public:
    SIM7080(const char *apn) : apn_(apn) {}

    // These methods should be called in order to initialize and use the modem
    void initialize();
    void startModem();
    void setupNetwork();

    // Perform an HTTPS POST request
    bool httpPost(const char *host, const char *path, const char *url, const char *data);

    // Get the current timestamp in milliseconds from the modem
    uint64_t getCurrentTimestampMs();

    // Ensure the modem is connected within the specified timeout (default 60 seconds)
    bool ensureConnected(uint32_t timeoutMs = 60000);

    // Read out battery level as a percentage (0-100)
    int getBatteryLevel();

private:
    const char *apn_;
};
