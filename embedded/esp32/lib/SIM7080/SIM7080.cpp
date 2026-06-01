#include "SIM7080.h"

#include <cstdio>

#include "utilities.h"

#define TINY_GSM_RX_BUFFER 1024  // Set RX buffer to 1Kb
#define SerialAT Serial1
#include <HelpMethod.h>

#define TINY_GSM_MODEM_SIM7080
#include <TinyGsmClient.h>
TinyGsm _modem{SerialAT};

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
XPowersPMU _pmu;

const char* register_info[] = {
    "Not registered, MT is not currently searching an operator to register to. "
    "The GPRS service is disabled, the UE is allowed to attach for GPRS if "
    "requested by the user.",
    "Registered, home network.",
    "Not registered, but MT is currently trying to attach or searching an "
    "operator to register to. The GPRS service is enabled, but an allowable "
    "PLMN is currently not available. The UE will start a GPRS attach as soon "
    "as an allowable PLMN is available.",
    "Registration denied, the GPRS service is disabled, the UE is not allowed "
    "to attach for GPRS if it is requested by the user.",
    "Unknown.",
    "Registered, roaming.",
};

enum {
    MODEM_CATM = 1,
    MODEM_NB_IOT,
    MODEM_CATM_NBIOT,
};

void readModemResponse(uint32_t timeout) {
    uint32_t start = millis();
    while (millis() - start < timeout) {
        while (Serial1.available()) {
            char c = Serial1.read();
            Serial.write(c);
            start = millis();  // reset timeout when data arrives
        }
    }
}

bool waitForHttpPostResult(int* statusCode, int* responseLength,
                           uint32_t timeout) {
    *statusCode = -1;
    *responseLength = -1;

    uint32_t start = millis();
    while (millis() - start < timeout) {
        while (Serial1.available()) {
            String line = Serial1.readStringUntil('\n');
            line.trim();

            if (line.length() == 0) {
                continue;
            }

            DEBUG_LOG_LN(line);
            start = millis();

            if (line.startsWith("+SHREQ:")) {
                int firstComma = line.indexOf(',');
                int secondComma = line.indexOf(',', firstComma + 1);

                if (firstComma < 0 || secondComma < 0) {
                    DEBUG_FAIL("Could not parse HTTPS POST result");
                    return false;
                }

                *statusCode =
                    line.substring(firstComma + 1, secondComma).toInt();
                *responseLength = line.substring(secondComma + 1).toInt();
                return true;
            }

            if (line == "ERROR") {
                DEBUG_FAIL(
                    "Modem returned ERROR while waiting for HTTPS POST result");
                return false;
            }
        }
    }

    DEBUG_FAIL("Timed out waiting for HTTPS POST result");
    return false;
}

bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int daysBeforeMonth(int year, int month) {
    static const int kDaysBeforeMonth[] = {
        0,    // January
        31,   // February
        59,   // March
        90,   // April
        120,  // May
        151,  // June
        181,  // July
        212,  // August
        243,  // September
        273,  // October
        304,  // November
        334,  // December
    };

    int days = kDaysBeforeMonth[month - 1];
    if (month > 2 && isLeapYear(year)) {
        days++;
    }
    return days;
}

int64_t utcEpochSecondsFromCivilTime(int year, int month, int day, int hour,
                                     int minute, int second) {
    int64_t days = 0;

    for (int currentYear = 1970; currentYear < year; currentYear++) {
        days += isLeapYear(currentYear) ? 366 : 365;
    }

    days += daysBeforeMonth(year, month);
    days += day - 1;

    return (((days * 24) + hour) * 60 + minute) * 60 + second;
}

void SIM7080::initialize() {
    // ----- Initialize Power Chip -----
    DEBUG_TRACE_SECTION("Initialize Power Chip");
    DEBUG_TRACE("Starting power chip...");

    if (!_pmu.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        DEBUG_FAIL("Failed to initialize power chip...");
        while (1) {
            wait(5000);
        }
    }

    _pmu.setVbusVoltageLimit(XPOWERS_AXP2101_VBUS_VOL_LIM_4V36);
    _pmu.setVbusCurrentLimit(XPOWERS_AXP2101_VBUS_CUR_LIM_1500MA);
    _pmu.setSysPowerDownVoltage(2600);

    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_RI_PIN, INPUT);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
    digitalWrite(BOARD_MODEM_DTR_PIN, LOW);

    // If it is a power cycle, turn off the modem power. Then restart it
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED) {
        _pmu.disableDC3();
        // Briefly let the modem rail drop before enabling it again.
        wait(200);
    }

    // BLDO1 powers level conversion between ESP32-S3 and SIM7080G UART.
    // LilyGO warns not to turn it off or modem communication will fail.
    _pmu.setBLDO1Voltage(3300);
    _pmu.enableBLDO1();

    // Without this, the AXP2101 can shut the whole board down when the SIM7080G
    // DC3 rail comes up. Keep Otii/current limiting in place when testing
    // changes.
    _pmu.disableDC3LowVoltageTurnOff();
    _pmu.setDC3LowVoltagePowerDowm(false);

    // SIM7080 modem power channel. LilyGO maps modem power to DC3 and
    // recommends keeping this rail at 3000 mV.
    _pmu.setDC3Voltage(3000);
    _pmu.enableDC3();

    // BLDO2 is only for the GNSS antenna rail. Leave it off for cellular data;
    // board bring-up tests showed it is not needed and can destabilize boot.

    // TS Pin detection must be disabled, otherwise it cannot be charged
    _pmu.disableTSPinMeasure();

    // Enable internal ADC detection
    _pmu.enableBattDetection();
    _pmu.enableVbusVoltageMeasure();
    _pmu.enableBattVoltageMeasure();
    _pmu.enableSystemVoltageMeasure();

    DEBUG_TRACE("Power chip initialized!");
}

void SIM7080::startModem() {
    // ----- Start Modem -----
    DEBUG_SECTION("Start Modem");

    Serial1.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);

    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_RI_PIN, INPUT);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
    digitalWrite(BOARD_MODEM_DTR_PIN, LOW);

    DEBUG_TRACE("Waiting for modem AT response...");

    int retry = 0;
    int powerCycleAttempts = 0;
    while (!_modem.testAT(1000)) {
        if (retry++ > 6) {
            DEBUG_WARN("Power cycling modem...");

            // Pull down PWRKEY for more than 1 second according to manual
            // requirements
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
            wait(100);
            digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
            wait(1000);
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);

            retry = 0;
            powerCycleAttempts++;

            DEBUG_WARN("Modem did not respond after AT retries, power-cycle attempt " +
                       String(powerCycleAttempts));
        }
    }

    DEBUG_OK("Modem started!");
    modemStarted_ = true;
}

void SIM7080::setupNetwork() {
    // ----- Setup Network -----
    DEBUG_TRACE_SECTION("Setup Network");
    DEBUG_TRACE("Setting up network...");

    if (_modem.getSimStatus() != SIM_READY) {
        DEBUG_FAIL("SIM Card is not inserted!!!");
        return;
    }

    // Disable RF
    _modem.sendAT("+CFUN=0");
    if (_modem.waitResponse(20000UL) != 1) {
        DEBUG_FAIL("Disable RF Failed!");
    }

    _modem.setNetworkMode(2);
    _modem.setPreferredMode(MODEM_CATM);

    // Set the APN manually using configured APN
    _modem.sendAT("+CGDCONT=1,\"IP\",\"", apn_, "\"");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Set operators apn Failed!");
        return;
    }

    // Enable RF
    _modem.sendAT("+CFUN=1");
    if (_modem.waitResponse(20000UL) != 1) {
        DEBUG_FAIL("Enable RF Failed!");
    }

    DEBUG_TRACE("Network setup completed!");

    // ----- Registering to Network -----
    DEBUG_SECTION("Registering to Network");

    DEBUG_INFO("Waiting for network registration...");
    SIM70xxRegStatus s;
    uint32_t tStart = millis();
    const uint32_t regTimeoutMs = 180000;  // 3 minutes timeout
    do {
        s = _modem.getRegistrationStatus();
        if (s != REG_OK_HOME && s != REG_OK_ROAMING) {
            if (millis() - tStart > regTimeoutMs) {
                DEBUG_FAIL("Network registration timeout");
                return;
            }
            wait(1000);
        }
    } while (s != REG_OK_HOME && s != REG_OK_ROAMING);
    DEBUG_TRACE_KV("Network register info:", register_info[s]);

    _modem.sendAT("+CNCFG=0,1,\"", apn_, "\"");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Config apn Failed!");
        return;
    }

    _modem.sendAT("+CNACT=0,1");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Activate network bearer Failed!");
        return;
    }

    bool res = _modem.isGprsConnected();
    DEBUG_TRACE_KV("GPRS status:", res ? "connected" : "not connected");

    _modem.sendAT("+CNACT?");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Query network bearer Failed!");
        return;
    }

    // Enable Local Time Stamp for getting network time
    _modem.sendAT(GF("+CLTS=1"));
    if (_modem.waitResponse(10000L) != 1) {
        DEBUG_FAIL("Enable Local Time Stamp Failed!");
        return;
    }

    // Before connecting, you need to confirm that the time has been
    // synchronized.
    _modem.sendAT("+CCLK?");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Get network time Failed!");
        return;
    }

    DEBUG_OK("Network registration completed!");
}

void SIM7080::shutdownForDeepSleep() {
    DEBUG_SECTION("SIM7080 Deep Sleep Shutdown");

    if (!_pmu.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        DEBUG_WARN("PMU unavailable, modem rails could not be disabled");
        return;
    }

    if (modemStarted_) {
        _modem.sendAT("+SMDISC");
        _modem.waitResponse(2000L);

        _modem.gprsDisconnect();

        _modem.sendAT("+CFUN=0");
        _modem.waitResponse(10000L);

        _modem.poweroff();
        _modem.waitResponse(5000L);
    }

    Serial1.end();
    modemStarted_ = false;

    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
    digitalWrite(BOARD_MODEM_DTR_PIN, LOW);
    pinMode(BOARD_MODEM_PWR_PIN, INPUT);
    pinMode(BOARD_MODEM_DTR_PIN, INPUT);
    pinMode(BOARD_MODEM_RXD_PIN, INPUT);
    pinMode(BOARD_MODEM_TXD_PIN, INPUT);

    DEBUG_INFO("Modem rails shutting down; ESP32 will enter deep sleep next");
    Serial.flush();
    wait(50);

    _pmu.disableDC3();
    _pmu.disableBLDO1();
    _pmu.disableVbusVoltageMeasure();
    _pmu.disableBattVoltageMeasure();
    _pmu.disableSystemVoltageMeasure();
    _pmu.disableBattDetection();

    DEBUG_OK("SIM7080 modem rails disabled for deep sleep");
}

bool SIM7080::ensureConnected(uint32_t timeoutMs) {
    uint32_t start = millis();
    if (_modem.isGprsConnected()) {
        return true;
    }
    DEBUG_SECTION("Reconnect");
    while (!_modem.isGprsConnected()) {
        if (millis() - start > timeoutMs) {
            DEBUG_FAIL("Reconnect timeout");
            return false;
        }
        DEBUG_INFO("Attempting to re-establish network...");
        initialize();
        startModem();
        setupNetwork();
        if (_modem.isGprsConnected()) {
            DEBUG_OK("Connection re-established");
            break;
        }
        wait(5000);
    }
    return true;
}

bool SIM7080::mqttPublish(const char* deviceId, const char* topic,
                          const char* payload) {
    DEBUG_TRACE_SECTION("SIM7080 MQTT Publish");
    DEBUG_TRACE_KV("MQTT topic", topic);

    _modem.sendAT("+SMCONF=\"URL\",\"", mqttHost_, "\",\"", mqttPort_, "\"");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("MQTT broker configuration failed");
        return false;
    }

    _modem.sendAT("+SMCONF=\"CLIENTID\",\"", deviceId, "\"");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("MQTT client ID configuration failed");
        return false;
    }

    DEBUG_TRACE("Connecting SIM MQTT client...");
    _modem.sendAT("+SMCONN");
    if (_modem.waitResponse(10000L) != 1) {
        DEBUG_FAIL("MQTT connect failed");
        return false;
    }
    DEBUG_TRACE("SIM MQTT connected");

    _modem.sendAT("+SMPUB=\"", topic, "\",", strlen(payload), ",0,0");
    if (_modem.waitResponse(">") != 1) {
        DEBUG_FAIL("MQTT publish prompt failed");
        return false;
    }

    SerialAT.write(payload);
    if (_modem.waitResponse(10000) != 1) {
        DEBUG_FAIL("MQTT publish failed");
        return false;
    }
    DEBUG_TRACE("SIM MQTT published");

    _modem.sendAT("+SMDISC");
    _modem.waitResponse();

    return true;
}

bool SIM7080::httpPost(const char* device_id, const char* signature,
                       const char* host, const char* path, const char* url,
                       const char* data) {
    DEBUG_SECTION("Starting HTTPS POST request");

    if (!_modem.isGprsConnected()) {
        DEBUG_FAIL("GPRS is not connected!");
        if (!ensureConnected()) {
            return false;
        }
    }

    _modem.sendAT("+CSSLCFG=\"sslversion\",1,3");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Set SSL version Failed!");
        return false;
    }

    _modem.sendAT("+CSSLCFG=\"sni\",1,\"", host, "\"");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Set SNI Failed!");
        return false;
    }

    _modem.sendAT("+SHSSL=1,\"\"");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Set SHSSL Failed!");
        return false;
    }

    _modem.sendAT("+SHCONF=\"URL\",\"", url, "\"");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Set URL Failed!");
        return false;
    }

    // Set BODYLEN and HEADERLEN
    _modem.sendAT("+SHCONF=\"BODYLEN\",1024");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Set BODYLEN Failed!");
        return false;
    }

    _modem.sendAT("+SHCONF=\"HEADERLEN\",350");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Set HEADERLEN Failed!");
        return false;
    }

    // Connect HTTPS
    _modem.sendAT("+SHCONN");
    if (_modem.waitResponse(10000L) != 1) {
        DEBUG_FAIL("HTTPS Connection Failed!");
        return false;
    }

    // Optional state check
    _modem.sendAT("+SHSTATE?");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("HTTPS State Query Failed!");
        // Continue anyway
    }

    // Clear headers
    _modem.sendAT("+SHCHEAD");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Clear headers Failed!");
        return false;
    }

    // Set required headers
    _modem.sendAT("+SHAHEAD=\"Content-Type\",\"application/json\"");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Set Content-Type header Failed!");
        return false;
    }

    // Set custom authentication headers
    _modem.sendAT("+SHAHEAD=\"X-Device-ID\",\"", device_id, "\"");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Set X-Device-ID header Failed!");
        return false;
    }

    _modem.sendAT("+SHAHEAD=\"X-Signature\",\"", signature, "\"");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Set X-Signature header Failed!");
        return false;
    }

    _modem.sendAT("+SHAHEAD=\"Connection\",\"keep-alive\"");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Set Connection header Failed!");
        return false;
    }

    _modem.sendAT("+SHAHEAD=\"Accept\",\"*/*\"");
    if (_modem.waitResponse() != 1) {
        DEBUG_FAIL("Set Accept header Failed!");
        return false;
    }

    int bodyLen = strlen(data);

    // Write body
    _modem.sendAT("+SHBOD=", bodyLen, ",10000");
    if (_modem.waitResponse(">") != 1) {
        DEBUG_FAIL("Modem not ready to accept body data");
        return false;
    }

    SerialAT.write(data);

    if (_modem.waitResponse(10000) != 1) {
        DEBUG_FAIL("Sending body failed");
        return false;
    }

    // Trigger POST
    _modem.sendAT("+SHREQ=\"", path, "\",3");
    int statusCode = -1;
    int responseLength = -1;
    if (!waitForHttpPostResult(&statusCode, &responseLength, 15000)) {
        DEBUG_FAIL("POST failed");
        return false;
    }

    DEBUG_KV("HTTPS POST status", statusCode);
    DEBUG_KV("HTTPS POST response length", responseLength);

    // Read some of the response for visibility
    if (responseLength > 0) {
        int bytesToRead = responseLength > 512 ? 512 : responseLength;
        _modem.sendAT("+SHREAD=0,", bytesToRead);
        readModemResponse(2000);
    }

    // Disconnect HTTPS
    _modem.sendAT("+SHDISC");
    if (_modem.waitResponse(10000L) != 1) {
        DEBUG_FAIL("HTTPS Disconnect Failed!");
        // Continue anyway
    }

    if (statusCode < 200 || statusCode >= 300) {
        if (statusCode == 401) {
            DEBUG_FAIL(
                "Server rejected device credentials or signature (401 "
                "Unauthorized)");
        }

        DEBUG_FAIL("HTTPS POST failed with status " + String(statusCode));
        return false;
    }

    DEBUG_OK("HTTPS POST request completed");
    return true;
}

uint64_t SIM7080::getCurrentTimestampMs() {
    _modem.sendAT("+CCLK?");

    String cclkLine;
    uint32_t start = millis();
    while (millis() - start < 5000) {  // 5s timeout
        if (_modem.stream.available()) {
            String line = _modem.stream.readStringUntil('\n');
            line.trim();
            if (line.startsWith("+CCLK:")) {
                cclkLine = line;
            } else if (line == "OK") {  // end of response
                break;
            }
        }
    }

    if (cclkLine.length() == 0) {
        DEBUG_FAIL("No +CCLK line found");
        return 0;
    }

    int year, month, day, hour, min, sec;
    char tzSignChar;
    int tzOffset;

    // Parse +CCLK: "yy/MM/dd,hh:mm:ss±zz"
    if (sscanf(cclkLine.c_str(), "+CCLK: \"%2d/%2d/%2d,%2d:%2d:%2d%1c%2d",
               &year, &month, &day, &hour, &min, &sec, &tzSignChar,
               &tzOffset) != 8) {
        DEBUG_FAIL("Failed to parse CCLK with timezone");
        return 0;
    }

    // Convert timezone offset to minutes
    int tzMult = (tzSignChar == '-' ? -1 : 1);
    int tzMinutes =
        tzMult * tzOffset * 15;  // SIM7080 tz offset is in 15-min increments

    int fullYear = year + 2000;
    int64_t timestampSeconds =
        utcEpochSecondsFromCivilTime(fullYear, month, day, hour, min, sec) -
        (static_cast<int64_t>(tzMinutes) * 60);

    if (timestampSeconds < 0) {
        DEBUG_FAIL("Parsed CCLK time is before Unix epoch");
        return 0;
    }

    return static_cast<uint64_t>(timestampSeconds) * 1000;
}

int SIM7080::getBatteryVoltage() {
    return _pmu.getBattVoltage();
}

int SIM7080::getBatteryLevel() {
    return _pmu.getBatteryPercent();
}
