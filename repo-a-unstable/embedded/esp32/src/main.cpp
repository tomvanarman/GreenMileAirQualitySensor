// External libraries
#include <Arduino.h>
#include <Handler.h>
#include <Wire.h>
#include <math.h>
#include <sys/time.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <deque>
#include <string>

// Custom structured debug logging
#include "DEBUG.h"

// Utility Libraries
#include <JsonBuilder.h>

#include "ColorMap.h"
#include "HelpMethod.h"

// Sensor Libraries
#include "RGBLight.h"
#include "SHT41Sensor.h"
#include "SPS30.h"

// Actuator Libraries
#include "LEDStrip.h"
#include "SegmentDisplay.h"

// SIM7080 Modem Library
#include "SIM7080.h"

// Network Libraries
#include "CredentialManager.h"
#include "HttpManager.h"
#include "NetworkServer.h"
#include "WifiManager.h"
#include "esp_event.h"
#include "mqtt_client.h"

bool useSIM = false;
bool simInitialized = false;

// Server configuration for HTTPS POST
const char* host = "greenmile.tapp.city";
const char* url = "https://greenmile.tapp.city";
const char* SPS30path = "/api/data/sps30";
const char* SHT41path = "/api/data/sht41";
const char* BatteryPath = "/api/data/battery";

// Server config for MQTT
const char* MQTT_URI = "mqtt://greenmile.tapp.city";
const char* MQTT_HOST = "greenmile.tapp.city";
uint32_t MQTT_PORT = 1883;

// const char* MQTT_TOPIC    = "climate-box/#"; //doesnt need to listen to
// topics, only publish
char MQTT_PUBLISH_TOPIC_SPS30[64];
char MQTT_PUBLISH_TOPIC_SHT41[64];
char MQTT_PUBLISH_TOPIC_BATTERY[64];

// Timing constants
constexpr uint32_t kReconnectInterval = 6000;
constexpr uint32_t kDataTransmissionInterval = 10000;
constexpr uint32_t kDataMeasurementInterval = 2000;
constexpr int kPayloadSendAttempts = 2;
constexpr int kSensorSdaPin = 8;
constexpr int kSensorSclPin = 9;
constexpr uint32_t kSensorI2cClockHz = 50000;
constexpr uint32_t kSensorI2cTimeoutMs = 1000;
constexpr uint32_t kSensorPowerSettleMs = 10000;
constexpr uint32_t kSensorBusReadyTimeoutMs = 90000;
constexpr uint8_t kRequiredStableSps30Readings = 1;

// Global objects
SIM7080 sim7080("iot.1nce.net", MQTT_HOST,
                MQTT_PORT);  // APN for 1NCE IoT SIM cards

CredentialManager credential_manager;
NetworkServer server(credential_manager);
WiFiManager network(credential_manager);
HttpManager httpManager;
esp_mqtt_client_handle_t client = nullptr;

TwoWire WireSensors = TwoWire(1);

SPS30 sps30;
SHT41Sensor sht41;
Handler handler;

LEDStrip strip;
SegmentDisplay segmentDisplay(11, 12, 10);  // Data, CLK, CS pins
RGBLight rgbLight(14, 13, 12);
bool rgbLightReady = false;

void debugWarnIndicator() {
    if (rgbLightReady)
        rgbLight.warningBlink();
}

SHT41Data lastGoodSht41Data = {0, 0, false};
SPS30_measurement lastGoodSps30Data{};
uint8_t stableSps30Readings = 0;
bool sps30SentThisWake = false;
bool sht41SentThisWake = false;

// For debugging purposes
float maxPM = 25.0f;
GradientStop stops[] = {
    {0.0f / maxPM, 0, 255, 0},     // green
    {5.0f / maxPM, 255, 255, 0},   // yellow
    {10.0f / maxPM, 255, 165, 0},  // orange
    {15.0f / maxPM, 255, 0, 0},    // red
    {25.0f / maxPM, 128, 0, 0},    // maroon
};

ColorMap colorMap(maxPM, stops);

// NTP server configuration
const char* ntpServer1 = "149.143.87.22";  // pool.ntp.org
const char* ntpServer2 = "82.65.248.56";   // europe.pool.ntp.org
const char* localTimeZone = "CET-1CEST,M3.5.0/2,M10.5.0/3";

// Function declarations
bool HandleSPS30Logic();
bool HandleSHT41Logic();
bool HandleBatteryLogic();
bool sendPayload(const char* path, const String& payload);
const char* getMqttTopicForPath(const char* path);
bool initializeMqttClient();
void initializeTime();
void configureLocalTimezone();
void syncSystemTimeFromSIM();
bool switchToSIMNetwork();
uint64_t getCurrentTimestampMs();
int determineBatteryLevel();
void scanKnownI2CDevices(TwoWire* wire);
void initializeSensorI2CBus();
void recoverSensorI2CBus();
bool waitForKnownI2CDevices(TwoWire* wire, uint32_t timeoutMs);
bool isValidSPS30Measurement(const SPS30_measurement& data);
bool isValidSHT41Measurement(const SHT41Data& data);
static void mqtt_event_handler(void* handler_args, esp_event_base_t base,
                               int32_t event_id, void* event_data);

void setup() {
    Serial.begin(115200);
    configureLocalTimezone();
    Serial.println();
    Serial.println("====================================");
    Serial.println("Starting GreenMile Air Quality Sensor");

    wait(5000);  // Give some time to open the serial monitor after reset

    DEBUG_SECTION("Setup");

    strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BREATHING);
    handler.setupRGB(rgbLight);
    rgbLightReady = true;
    handler.rgbInitialization(rgbLight);

    handler.setupCredentialManager(credential_manager, server, rgbLight);

    snprintf(MQTT_PUBLISH_TOPIC_SPS30, sizeof(MQTT_PUBLISH_TOPIC_SPS30),
             "greenmile/%s/sps30/data",
             credential_manager.GetDeviceID().c_str());
    snprintf(MQTT_PUBLISH_TOPIC_SHT41, sizeof(MQTT_PUBLISH_TOPIC_SHT41),
             "greenmile/%s/sht41/data",
             credential_manager.GetDeviceID().c_str());
    snprintf(MQTT_PUBLISH_TOPIC_BATTERY, sizeof(MQTT_PUBLISH_TOPIC_BATTERY),
             "greenmile/%s/battery/data",
             credential_manager.GetDeviceID().c_str());

    if (credential_manager.ValidateWifiCredentials()) {
        //============================================================================
        // Setup WiFi first. SIM is used as the fallback transport.
        //============================================================================
        network.Connect();

        if (network.isConnected()) {
            useSIM = false;
            initializeTime();
            initializeMqttClient();
        } else {
            DEBUG_WARN("WiFi unavailable, falling back to SIM7080");
            switchToSIMNetwork();
        }
    } else {
        DEBUG_WARN("No WiFi credentials configured, using SIM7080");
        switchToSIMNetwork();
    }

    strip.stopLoading();
    strip.clear();
    handler.disableRGB(rgbLight);

    initializeSensorI2CBus();

    //============================================================================
    // Setup SPS30
    //============================================================================
    handler.setupSPS30(sps30, WireSensors, rgbLight);

    //============================================================================
    // Setup SHT41
    //============================================================================
    handler.setupSHT41(sht41, WireSensors, rgbLight);

    strip.clear();

    // Initialize LUT for color mapping
    colorMap.InitLUT();

    // Initialize segment display
    segmentDisplay.start();
    // int batteryLevel = determineBatteryLevel();
    // segmentDisplay.setBattery(batteryLevel);
    // wait(3000);
    segmentDisplay.clearDisplay();
}

void loop() {
    Serial.println("Loop start");
    Serial.println("=====================================");

    if (!sps30SentThisWake) {
        sps30SentThisWake = HandleSPS30Logic();
    }

    if (!sht41SentThisWake) {
        sht41SentThisWake = HandleSHT41Logic();
    }
    // bool batterySent = HandleBatteryLogic();

    if (sps30SentThisWake && sht41SentThisWake) {
        DEBUG_OK("All data sent successfully, entering deep sleep");
        wait(500);
        handler.enterDeepSleep(strip, segmentDisplay, sps30, WireSensors,
                               sim7080, kSensorSdaPin, kSensorSclPin, useSIM);
    }

    wait(250);
}

bool probeI2CAddress(TwoWire* wire, uint8_t address) {
    wire->beginTransmission(address);
    return wire->endTransmission() == 0;
}

void probeI2CDevice(TwoWire* wire, uint8_t address, const char* name) {
    wire->beginTransmission(address);
    uint8_t error = wire->endTransmission();

    String addressText = "0x";
    if (address < 16) {
        addressText += "0";
    }
    addressText += String(address, HEX);
    addressText.toUpperCase();

    if (error == 0) {
        DEBUG_OK(String(name) + " found at " + addressText);
        return;
    }

    DEBUG_WARN(String(name) + " missing at " + addressText + ", error " +
               String(error));
}

void scanKnownI2CDevices(TwoWire* wire) {
    DEBUG_SECTION("I2C Known Device Probe");
    probeI2CDevice(wire, 0x44, "SHT41");
    probeI2CDevice(wire, 0x69, "SPS30");
}

void recoverSensorI2CBus() {
    DEBUG_SECTION("Sensor I2C Bus Recovery");

    WireSensors.end();

    pinMode(kSensorSdaPin, INPUT_PULLUP);
    pinMode(kSensorSclPin, INPUT_PULLUP);
    wait(10);

    bool sdaHigh = digitalRead(kSensorSdaPin) == HIGH;
    bool sclHigh = digitalRead(kSensorSclPin) == HIGH;
    DEBUG_KV("SDA idle", sdaHigh ? "HIGH" : "LOW");
    DEBUG_KV("SCL idle", sclHigh ? "HIGH" : "LOW");

    if (!sdaHigh) {
        DEBUG_WARN("SDA is low, pulsing SCL to release the I2C bus");
        pinMode(kSensorSclPin, OUTPUT_OPEN_DRAIN);

        for (uint8_t pulse = 0; pulse < 9; pulse++) {
            digitalWrite(kSensorSclPin, LOW);
            delayMicroseconds(5);
            digitalWrite(kSensorSclPin, HIGH);
            delayMicroseconds(5);
        }
    }

    pinMode(kSensorSdaPin, INPUT_PULLUP);
    pinMode(kSensorSclPin, INPUT_PULLUP);
}

bool waitForKnownI2CDevices(TwoWire* wire, uint32_t timeoutMs) {
    DEBUG_SECTION("Waiting for Sensor I2C Devices");

    uint32_t start = millis();
    uint32_t attempt = 1;

    while (millis() - start < timeoutMs) {
        bool shtFound = probeI2CAddress(wire, 0x44);
        bool spsFound = probeI2CAddress(wire, 0x69);

        if (shtFound && spsFound) {
            DEBUG_OK("SHT41 and SPS30 are responding on I2C");
            return true;
        }

        DEBUG_WARN("Sensor I2C wait attempt " + String(attempt) +
                   " SHT41=" + String(shtFound ? "OK" : "missing") +
                   " SPS30=" + String(spsFound ? "OK" : "missing"));
        attempt++;
        wait(2000);
    }

    DEBUG_WARN("Sensor I2C devices did not both respond before timeout");
    return false;
}

void initializeSensorI2CBus() {
    DEBUG_SECTION("Sensor I2C Setup");
    DEBUG_KV("Sensor SDA pin", kSensorSdaPin);
    DEBUG_KV("Sensor SCL pin", kSensorSclPin);

    recoverSensorI2CBus();

    WireSensors.begin(kSensorSdaPin, kSensorSclPin);
    WireSensors.setClock(kSensorI2cClockHz);
    WireSensors.setTimeOut(kSensorI2cTimeoutMs);

    sps30.wakeUp(WireSensors);

    DEBUG_INFO("Waiting for sensor power-up");
    wait(kSensorPowerSettleMs);

    scanKnownI2CDevices(&WireSensors);
    waitForKnownI2CDevices(&WireSensors, kSensorBusReadyTimeoutMs);
}

bool isFiniteNonNegative(float value) {
    return isfinite(value) && value >= 0.0f;
}

bool isValidSPS30Measurement(const SPS30_measurement& data) {
    constexpr float kMaxMassConcentration = 1000.0f;
    constexpr float kMaxNumberConcentration = 100000.0f;
    constexpr float kMinParticleSize = 0.1f;
    constexpr float kMaxParticleSize = 10.0f;

    if (!isFiniteNonNegative(data.mc_1p0) ||
        !isFiniteNonNegative(data.mc_2p0) ||
        !isFiniteNonNegative(data.mc_4p0) ||
        !isFiniteNonNegative(data.mc_10p0) ||
        !isFiniteNonNegative(data.nc_0p5) ||
        !isFiniteNonNegative(data.nc_1p0) ||
        !isFiniteNonNegative(data.nc_2p5) ||
        !isFiniteNonNegative(data.nc_4p0) ||
        !isFiniteNonNegative(data.nc_10p0) ||
        !isfinite(data.typical_particle_size)) {
        return false;
    }

    if (data.mc_1p0 > kMaxMassConcentration ||
        data.mc_2p0 > kMaxMassConcentration ||
        data.mc_4p0 > kMaxMassConcentration ||
        data.mc_10p0 > kMaxMassConcentration ||
        data.nc_0p5 > kMaxNumberConcentration ||
        data.nc_1p0 > kMaxNumberConcentration ||
        data.nc_2p5 > kMaxNumberConcentration ||
        data.nc_4p0 > kMaxNumberConcentration ||
        data.nc_10p0 > kMaxNumberConcentration) {
        return false;
    }

    return data.typical_particle_size >= kMinParticleSize &&
           data.typical_particle_size <= kMaxParticleSize;
}

bool isValidSHT41Measurement(const SHT41Data& data) {
    return isfinite(data.temperature) && isfinite(data.humidity) &&
           data.temperature >= -40.0f && data.temperature <= 125.0f &&
           data.humidity >= 0.0f && data.humidity <= 100.0f;
}

bool HandleSPS30Logic() {
    uint32_t now = millis();
    bool can_measure = now - sps30.last_measurement >= kDataMeasurementInterval;

    if (!can_measure)
        return false;

    if (!sps30.isInitialized()) {
        DEBUG_WARN("SPS30 not initialized, retrying setup");
        sps30.begin(WireSensors, 1);
        return false;
    }

    SPS30_measurement spsData = sps30.readData();

    if (!spsData.available) {
        DEBUG_WARN("Failed to read from sps30 sensor");

        if (!strip.isLoading())
            strip.startLoading(CRGB::DeepSkyBlue,
                               LEDStrip::_loadingModeType::BLINKING);

        return false;
    }

    if (!isValidSPS30Measurement(spsData)) {
        DEBUG_WARN("Rejected invalid SPS30 measurement");
        DEBUG_KV("SPS30 rejected PM2.5", spsData.mc_2p0);
        DEBUG_KV("SPS30 rejected PM10", spsData.mc_10p0);
        DEBUG_KV("SPS30 rejected size", spsData.typical_particle_size);
        sps30.debug_raw_next_read = true;
        stableSps30Readings = 0;
        return false;
    }

    lastGoodSps30Data = spsData;
    stableSps30Readings++;

    if (stableSps30Readings < kRequiredStableSps30Readings) {
        DEBUG_WARN("Waiting for stable SPS30 readings");
        sps30.last_measurement = now;
        return false;
    }

    if (strip.isLoading())
        strip.stopLoading();

    sps30.last_measurement = now;

    // Update LED strip based on PM2.5 value
    Color c = colorMap.DataToColor(spsData.mc_2p0);
    strip.toColor(CRGB(c.r, c.g, c.b), 50);

    sps30.last_update = now;

    // Pretty print for debug
    String prettyData = JsonBuilder::BuildJson(
        [&](JsonDocument& doc) {
            doc["mc_1p0"] = spsData.mc_1p0;
            doc["mc_2p0"] = spsData.mc_2p0;
            doc["mc_4p0"] = spsData.mc_4p0;
            doc["mc_10p0"] = spsData.mc_10p0;
            doc["nc_0p5"] = spsData.nc_0p5;
            doc["nc_1p0"] = spsData.nc_1p0;
            doc["nc_2p5"] = spsData.nc_2p5;
            doc["nc_4p0"] = spsData.nc_4p0;
            doc["nc_10p0"] = spsData.nc_10p0;
            doc["typical_particle_size"] = spsData.typical_particle_size;

            if (useSIM)
                doc["time_unix"] = sim7080.getCurrentTimestampMs() / 1000;
            else
                doc["time_unix"] = getCurrentTimestampMs() / 1000;
        },
        true);

    DEBUG_BLOCK("Payload");
    DEBUG_INFO(prettyData);

    // Compact version for actual POST
    String data = JsonBuilder::BuildJson([&](JsonDocument& doc) {
        doc["mc_1p0"] = spsData.mc_1p0;
        doc["mc_2p0"] = spsData.mc_2p0;
        doc["mc_4p0"] = spsData.mc_4p0;
        doc["mc_10p0"] = spsData.mc_10p0;
        doc["nc_0p5"] = spsData.nc_0p5;
        doc["nc_1p0"] = spsData.nc_1p0;
        doc["nc_2p5"] = spsData.nc_2p5;
        doc["nc_4p0"] = spsData.nc_4p0;
        doc["nc_10p0"] = spsData.nc_10p0;
        doc["typical_particle_size"] = spsData.typical_particle_size;

        if (useSIM)
            doc["time_unix"] = sim7080.getCurrentTimestampMs() / 1000;
        else
            doc["time_unix"] = getCurrentTimestampMs() / 1000;
    });

    bool sent = sendPayload(SPS30path, data);

    if (sent) {
        DEBUG_INFO("Successfully sent SPS30 data");
        return true;
    }

    DEBUG_WARN("Failed to send SPS30 data");
    return false;
}

// Read out the SHT41 sensor
bool HandleSHT41Logic() {
    uint32_t now = millis();
    bool can_measure = now - sht41.last_measurement >= kDataMeasurementInterval;

    if (!can_measure)
        return false;

    if (!sht41.isInitialized()) {
        DEBUG_WARN("SHT41 not initialized, retrying setup");
        sht41.begin(WireSensors, 1);
        return false;
    }

    SHT41Data shtData = sht41.readData(3);

    if (!shtData.available) {
        DEBUG_WARN("Failed to read from SHT41 sensor");
        segmentDisplay.setError();
        if (lastGoodSht41Data.available) {
            DEBUG_WARN("Keeping last good SHT41 reading");
            sht41.last_measurement = now;
        }
        return false;
    }

    if (!isValidSHT41Measurement(shtData)) {
        DEBUG_WARN("Rejected invalid SHT41 measurement");
        segmentDisplay.setError();
        return false;
    }

    segmentDisplay.setTemp(shtData.temperature);
    segmentDisplay.setHum(shtData.humidity);

    lastGoodSht41Data = shtData;
    sht41.last_measurement = now;

    sht41.last_update = now;

    // Pretty print for debug
    String prettyData = JsonBuilder::BuildJson(
        [&](JsonDocument& doc) {
            doc["temperature"] = shtData.temperature;
            doc["humidity"] = shtData.humidity;

            if (useSIM)
                doc["time_unix"] = sim7080.getCurrentTimestampMs() / 1000;
            else
                doc["time_unix"] = getCurrentTimestampMs() / 1000;
        },
        true);

    DEBUG_BLOCK("Payload");
    DEBUG_INFO(prettyData);

    // Compact version for actual POST
    String data = JsonBuilder::BuildJson([&](JsonDocument& doc) {
        doc["temperature"] = shtData.temperature;
        doc["humidity"] = shtData.humidity;

        if (useSIM)
            doc["time_unix"] = sim7080.getCurrentTimestampMs() / 1000;
        else
            doc["time_unix"] = getCurrentTimestampMs() / 1000;
    });

    bool sent = sendPayload(SHT41path, data);

    if (sent) {
        DEBUG_INFO("Successfully sent SHT41 data");
        return true;
    }

    DEBUG_WARN("Failed to send SHT41 data");
    return false;
}

// Read out the battery
bool HandleBatteryLogic() {
    uint32_t now = millis();
    bool can_measure =
        now - sim7080.last_battery_measurement >= kDataMeasurementInterval;
    bool can_update =
        now - sim7080.last_battery_update >= kDataTransmissionInterval;

    if (!can_measure)
        return false;

    int batteryLevel = determineBatteryLevel();

    sim7080.last_battery_measurement = now;

    if (!can_update)
        return false;

    sim7080.last_battery_update = now;

    // Pretty print for debug
    String prettyData = JsonBuilder::BuildJson(
        [&](JsonDocument& doc) {
            doc["level"] = batteryLevel;

            if (useSIM)
                doc["time_unix"] = sim7080.getCurrentTimestampMs() / 1000;
            else
                doc["time_unix"] = getCurrentTimestampMs() / 1000;
        },
        true);

    DEBUG_BLOCK("Payload");
    DEBUG_INFO(prettyData);

    // Compact version for actual POST
    String data = JsonBuilder::BuildJson([&](JsonDocument& doc) {
        doc["level"] = batteryLevel;

        if (useSIM)
            doc["time_unix"] = sim7080.getCurrentTimestampMs() / 1000;
        else
            doc["time_unix"] = getCurrentTimestampMs() / 1000;
    });

    bool sent = sendPayload(BatteryPath, data);

    if (sent) {
        DEBUG_INFO("Successfully sent Battery data");
        return true;
    }

    DEBUG_WARN("Failed to send Battery data");
    return false;
}

bool sendPayload(const char* path, const String& payload) {
    const char* topic = getMqttTopicForPath(path);
    if (topic == nullptr) {
        DEBUG_WARN("No MQTT topic configured for payload path");
        return false;
    }

    for (int attempt = 1; attempt <= kPayloadSendAttempts; attempt++) {
        DEBUG_KV("Payload send attempt", attempt);

        if (useSIM) {
            if (!sim7080.ensureConnected()) {
                DEBUG_WARN("Failed to connect to SIM network");
                strip.stopLoading();
                strip.startLoading(CRGB::Purple,
                                   LEDStrip::_loadingModeType::BLINKING);
            } else if (sim7080.mqttPublish(
                           credential_manager.GetDeviceID().c_str(), topic,
                           payload.c_str())) {
                return true;
            }
        } else {
            if (!network.isConnected()) {
                DEBUG_WARN("WiFi disconnected, reconnecting on WiFi");
                network.Connect();
            }

            if (network.isConnected()) {
                if (client == nullptr && !initializeMqttClient()) {
                    DEBUG_WARN("MQTT client is not ready");
                } else if (esp_mqtt_client_publish(
                               client, topic, payload.c_str(), 0, 1, 0) >= 0) {
                    return true;
                } else {
                    DEBUG_WARN("MQTT publish failed");
                }
            }
        }

        if (attempt < kPayloadSendAttempts) {
            wait(1000);
        }
    }

    return false;
}

const char* getMqttTopicForPath(const char* path) {
    if (strcmp(path, SPS30path) == 0)
        return MQTT_PUBLISH_TOPIC_SPS30;
    if (strcmp(path, SHT41path) == 0)
        return MQTT_PUBLISH_TOPIC_SHT41;
    if (strcmp(path, BatteryPath) == 0)
        return MQTT_PUBLISH_TOPIC_BATTERY;
    return nullptr;
}

bool initializeMqttClient() {
    if (client != nullptr)
        return true;

    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.host = MQTT_HOST;
    mqtt_cfg.uri = MQTT_URI;
    mqtt_cfg.port = MQTT_PORT;

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == nullptr) {
        DEBUG_WARN("Failed to initialize MQTT client");
        return false;
    }

    esp_mqtt_client_register_event(client, MQTT_EVENT_CONNECTED,
                                   mqtt_event_handler, nullptr);
    esp_mqtt_client_register_event(client, MQTT_EVENT_DATA, mqtt_event_handler,
                                   nullptr);
    esp_mqtt_client_register_event(client, MQTT_EVENT_DISCONNECTED,
                                   mqtt_event_handler, nullptr);

    esp_err_t startResult = esp_mqtt_client_start(client);
    if (startResult != ESP_OK) {
        DEBUG_WARN("Failed to start MQTT client");
        client = nullptr;
        return false;
    }

    return true;
}

bool switchToSIMNetwork() {
    useSIM = true;
    network.Disconnect();

    if (!simInitialized) {
        handler.setupSim7080(sim7080, rgbLight);
        simInitialized = true;
    }

    if (!sim7080.ensureConnected()) {
        DEBUG_WARN("SIM7080 network connection failed");
        return false;
    }

    syncSystemTimeFromSIM();
    return true;
}

void initializeTime() {
    DEBUG_SECTION("NTP Sync");
    configureLocalTimezone();
    configTzTime(localTimeZone, ntpServer1, ntpServer2);
    DEBUG_INFO("Waiting for NTP time sync...");
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo, 10000)) {
        DEBUG_WARN("Failed to obtain time, retrying...");
        wait(2000);
    }
    DEBUG_OK("Time synchronized");
    char timeBuffer[32];
    strftime(timeBuffer, sizeof(timeBuffer), "%a %b %d %H:%M:%S %Y", &timeinfo);
    DEBUG_KV("Current time", String(timeBuffer));
}

void configureLocalTimezone() {
    setenv("TZ", localTimeZone, 1);
    tzset();
}

uint64_t getCurrentTimestampMs() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return static_cast<uint64_t>(tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
}

int determineBatteryLevel() {
    int voltage = sim7080.getBatteryVoltage();
    int level = sim7080.getBatteryLevel();

    int maxAllowedLevel;

    if (voltage > 4100)
        maxAllowedLevel = 100;
    else if (voltage >= 4000)
        maxAllowedLevel = 75;
    else if (voltage >= 3900)
        maxAllowedLevel = 55;
    else if (voltage >= 3800)
        maxAllowedLevel = 35;
    else if (voltage >= 3700)
        maxAllowedLevel = 20;
    else
        maxAllowedLevel = 10;

    if (level < maxAllowedLevel) {
        level = maxAllowedLevel;
    }

    return level;
}

void syncSystemTimeFromSIM() {
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

static void mqtt_event_handler(void* handler_args, esp_event_base_t base,
                               int32_t event_id, void* event_data) {
    esp_mqtt_event_handle_t event =
        static_cast<esp_mqtt_event_handle_t>(event_data);

    switch (static_cast<esp_mqtt_event_id_t>(event_id)) {
        case MQTT_EVENT_CONNECTED:
            DEBUG_INFO("MQTT connected successfully");
            break;

        case MQTT_EVENT_DATA: {
            String topic;
            for (int i = 0; i < event->topic_len; i++)
                topic += static_cast<char>(event->topic[i]);

            String msg;
            for (int i = 0; i < event->data_len; i++)
                msg += static_cast<char>(event->data[i]);

            DEBUG_INFO("MQTT topic: " + topic);
            DEBUG_INFO("MQTT msg: " + msg);
            break;
        }

        case MQTT_EVENT_DISCONNECTED:
            DEBUG_WARN("MQTT disconnected from the broker");
            break;

        default:
            break;
    }
}
