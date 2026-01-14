#include "SIM7080.h"
#include "utilities.h"
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1

#define TINY_GSM_MODEM_SIM7080
#include <TinyGsmClient.h>
TinyGsm _modem{SerialAT};

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
XPowersPMU _pmu;

const char *register_info[] = {
    "Not registered, MT is not currently searching an operator to register to. The GPRS service is disabled, the UE is allowed to attach for GPRS if requested by the user.",
    "Registered, home network.",
    "Not registered, but MT is currently trying to attach or searching an operator to register to. The GPRS service is enabled, but an allowable PLMN is currently not available. The UE will start a GPRS attach as soon as an allowable PLMN is available.",
    "Registration denied, the GPRS service is disabled, the UE is not allowed to attach for GPRS if it is requested by the user.",
    "Unknown.",
    "Registered, roaming.",
};

enum
{
    MODEM_CATM = 1,
    MODEM_NB_IOT,
    MODEM_CATM_NBIOT,
};

void readModemResponse(uint32_t timeout)
{
    unsigned long start = millis();
    while (millis() - start < timeout)
    {
        while (Serial1.available())
        {
            char c = Serial1.read();
            Serial.write(c);
            start = millis(); // reset timeout when data arrives
        }
    }
}

void SIM7080::initialize()
{
    // ----- Initialize Power Chip -----
    DEBUG_SECTION("Initialize Power Chip");
    DEBUG_INFO("Starting power chip...");

    if (!_pmu.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL))
    {
        DEBUG_FAIL("Failed to initialize power chip...");
        while (1)
        {
            delay(5000);
        }
    }

    // If it is a power cycle, turn off the modem power. Then restart it
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED)
    {
        _pmu.disableDC3();
        // Wait a minute
        delay(200);
    }

    // Set the working voltage of the modem, please do not modify the parameters
    _pmu.setDC3Voltage(3000); // SIM7080 Modem main power channel 2700~ 3400V
    _pmu.enableDC3();

    // Modem GPS Power channel
    _pmu.setBLDO2Voltage(3300);
    _pmu.enableBLDO2(); // The antenna power must be turned on to use the GPS function

    // TS Pin detection must be disabled, otherwise it cannot be charged
    _pmu.disableTSPinMeasure();

    DEBUG_OK("Power chip initialized!");
}

void SIM7080::startModem()
{
    // ----- Start Modem -----
    DEBUG_SECTION("Start Modem");

    Serial1.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);

    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_RI_PIN, INPUT);

    DEBUG_INFO("Starting modem...");

    int retry = 0;
    while (!_modem.testAT(1000))
    {
        if (retry++ > 6)
        {
            // Pull down PWRKEY for more than 1 second according to manual requirements
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
            delay(100);
            digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
            delay(1000);
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);

            retry = 0;

            DEBUG_FAIL("Retrying to start modem...");
        }
    }

    DEBUG_OK("Modem started!");
}

void SIM7080::setupNetwork()
{
    // ----- Setup Network -----
    DEBUG_SECTION("Setup Network");
    DEBUG_INFO("Setting up network...");

    if (_modem.getSimStatus() != SIM_READY)
    {
        DEBUG_FAIL("SIM Card is not inserted!!!");
        return;
    }

    // Disable RF
    _modem.sendAT("+CFUN=0");
    if (_modem.waitResponse(20000UL) != 1)
    {
        DEBUG_FAIL("Disable RF Failed!");
    }

    _modem.setNetworkMode(2);
    _modem.setPreferredMode(MODEM_CATM);

    // Set the APN manually using configured APN
    _modem.sendAT("+CGDCONT=1,\"IP\",\"", apn_, "\"");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("Set operators apn Failed!");
        return;
    }

    // Enable RF
    _modem.sendAT("+CFUN=1");
    if (_modem.waitResponse(20000UL) != 1)
    {
        DEBUG_FAIL("Enable RF Failed!");
    }

    DEBUG_OK("Network setup completed!");

    // ----- Registering to Network -----
    DEBUG_SECTION("Registering to Network");

    DEBUG_INFO("Waiting for network registration...");
    SIM70xxRegStatus s;
    uint32_t tStart = millis();
    const uint32_t regTimeoutMs = 180000; // 3 minutes timeout
    do
    {
        s = _modem.getRegistrationStatus();
        if (s != REG_OK_HOME && s != REG_OK_ROAMING)
        {
            if (millis() - tStart > regTimeoutMs)
            {
                DEBUG_FAIL("Network registration timeout");
                return;
            }
            delay(1000);
        }
    } while (s != REG_OK_HOME && s != REG_OK_ROAMING);
    DEBUG_KV("Network register info:", register_info[s]);

    _modem.sendAT("+CNCFG=0,1,\"", apn_, "\"");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("Config apn Failed!");
        return;
    }

    _modem.sendAT("+CNACT=0,1");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("Activate network bearer Failed!");
        return;
    }

    bool res = _modem.isGprsConnected();
    DEBUG_KV("GPRS status:", res ? "connected" : "not connected");

    _modem.sendAT("+CNACT?");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("Query network bearer Failed!");
        return;
    }

    // Enable Local Time Stamp for getting network time
    _modem.sendAT(GF("+CLTS=1"));
    if (_modem.waitResponse(10000L) != 1)
    {
        DEBUG_FAIL("Enable Local Time Stamp Failed!");
        return;
    }

    // Before connecting, you need to confirm that the time has been synchronized.
    _modem.sendAT("+CCLK?");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("Get network time Failed!");
        return;
    }

    DEBUG_OK("Network registration completed!");
}

bool SIM7080::ensureConnected(uint32_t timeoutMs)
{
    unsigned long start = millis();
    if (_modem.isGprsConnected())
    {
        return true;
    }
    DEBUG_SECTION("Reconnect");
    while (!_modem.isGprsConnected())
    {
        if (millis() - start > timeoutMs)
        {
            DEBUG_FAIL("Reconnect timeout");
            return false;
        }
        DEBUG_INFO("Attempting to re-establish network...");
        initialize();
        startModem();
        setupNetwork();
        if (_modem.isGprsConnected())
        {
            DEBUG_OK("Connection re-established");
            break;
        }
        delay(5000);
    }
    return true;
}

bool SIM7080::httpPost(const char *host, const char *path, const char *url, const char *data)
{
    DEBUG_SECTION("Starting HTTPS POST request");

    if (!_modem.isGprsConnected())
    {
        DEBUG_FAIL("GPRS is not connected!");
        if (!ensureConnected())
        {
            return false;
        }
    }

    _modem.sendAT("+CSSLCFG=\"sslversion\",1,3");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("Set SSL version Failed!");
        return false;
    }

    _modem.sendAT("+CSSLCFG=\"sni\",1,\"", host, "\"");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("Set SNI Failed!");
        return false;
    }

    _modem.sendAT("+SHSSL=1,\"\"");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("Set SHSSL Failed!");
        return false;
    }

    _modem.sendAT("+SHCONF=\"URL\",\"", url, "\"");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("Set URL Failed!");
        return false;
    }

    // Set BODYLEN and HEADERLEN
    _modem.sendAT("+SHCONF=\"BODYLEN\",1024");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("Set BODYLEN Failed!");
        return false;
    }

    _modem.sendAT("+SHCONF=\"HEADERLEN\",350");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("Set HEADERLEN Failed!");
        return false;
    }

    // Connect HTTPS
    _modem.sendAT("+SHCONN");
    if (_modem.waitResponse(10000L) != 1)
    {
        DEBUG_FAIL("HTTPS Connection Failed!");
        return false;
    }

    // Optional state check
    _modem.sendAT("+SHSTATE?");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("HTTPS State Query Failed!");
        // Continue anyway
    }

    // Clear headers
    _modem.sendAT("+SHCHEAD");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("Clear headers Failed!");
        return false;
    }

    // Set required headers
    _modem.sendAT("+SHAHEAD=\"Content-Type\",\"application/json\"");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("Set Content-Type header Failed!");
        return false;
    }

    _modem.sendAT("+SHAHEAD=\"Connection\",\"keep-alive\"");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("Set Connection header Failed!");
        return false;
    }

    _modem.sendAT("+SHAHEAD=\"Accept\",\"*/*\"");
    if (_modem.waitResponse() != 1)
    {
        DEBUG_FAIL("Set Accept header Failed!");
        return false;
    }

    int bodyLen = strlen(data);

    // Write body
    _modem.sendAT("+SHBOD=", bodyLen, ",10000");
    if (_modem.waitResponse(">") != 1)
    {
        DEBUG_FAIL("Modem not ready to accept body data");
        return false;
    }

    SerialAT.write(data);

    if (_modem.waitResponse(10000) != 1)
    {
        DEBUG_FAIL("Sending body failed");
        return false;
    }

    // Trigger POST
    _modem.sendAT("+SHREQ=\"", path, "\",3");
    if (_modem.waitResponse(15000, "+SHREQ: ") != 1)
    {
        DEBUG_FAIL("POST failed");
        return false;
    }

    // Read some of the response for visibility
    _modem.sendAT("+SHREAD=0,512");
    readModemResponse(2000);

    // Disconnect HTTPS
    _modem.sendAT("+SHDISC");
    if (_modem.waitResponse(10000L) != 1)
    {
        DEBUG_FAIL("HTTPS Disconnect Failed!");
        // Continue anyway
    }

    DEBUG_INFO("HTTPS POST request completed!");
    return true;
}

uint64_t SIM7080::getCurrentTimestampMs()
{
    _modem.sendAT("+CCLK?");

    String cclkLine;
    unsigned long start = millis();
    while (millis() - start < 5000)  // 5s timeout
    {
        if (_modem.stream.available())
        {
            String line = _modem.stream.readStringUntil('\n');
            line.trim();
            if (line.startsWith("+CCLK:"))
            {
                cclkLine = line;
            }
            else if (line == "OK")  // end of response
            {
                break;
            }
        }
    }

    if (cclkLine.length() == 0)
    {
        DEBUG_FAIL("No +CCLK line found");
        return 0;
    }

    int year, month, day, hour, min, sec;
    char tzSignChar;
    int tzOffset;

    // Parse +CCLK: "yy/MM/dd,hh:mm:ss±zz"
    if (sscanf(cclkLine.c_str(), "+CCLK: \"%2d/%2d/%2d,%2d:%2d:%2d%1c%2d",
               &year, &month, &day, &hour, &min, &sec, &tzSignChar, &tzOffset) != 8)
    {
        DEBUG_FAIL("Failed to parse CCLK with timezone");
        return 0;
    }

    // Convert timezone offset to minutes
    int tzMult = (tzSignChar == '-' ? -1 : 1);
    int tzMinutes = tzMult * tzOffset * 15; // SIM7080 tz offset is in 15-min increments

    // Fill tm structure
    struct tm timeinfo;
    timeinfo.tm_year = year + 2000 - 1900;
    timeinfo.tm_mon  = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min  = min - tzMinutes; // convert to UTC
    timeinfo.tm_sec  = sec;
    timeinfo.tm_isdst = 0;

    time_t t = mktime(&timeinfo);
    if (t == -1)
    {
        DEBUG_FAIL("mktime failed");
        return 0;
    }

    return static_cast<uint64_t>(t) * 1000;
}

int SIM7080::getBatteryLevel()
{
    DEBUG_LOG("Battery level:" + String(_pmu.getBatteryPercent()));
    return _pmu.getBatteryPercent();
}