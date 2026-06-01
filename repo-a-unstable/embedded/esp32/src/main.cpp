#include <Arduino.h>
#include <Handler.h>
#include <Wire.h>

#include "CredentialManager.h"
#include "DEBUG.h"
#include "HelpMethod.h"
#include "NetworkServer.h"
#include "PayloadPublisher.h"
#include "RGBLight.h"
#include "SHT41Sensor.h"
#include "SIM7080.h"
#include "SPS30.h"
#include "SensorReadoutManager.h"
#include "TimeService.h"
#include "WifiManager.h"

namespace {
constexpr const char* kSimApn = "iot.1nce.net";
constexpr const char* kMqttHost = "greenmile.tapp.city";
constexpr uint32_t kMqttPort = 1883;

constexpr int kSensorSdaPin = 8;
constexpr int kSensorSclPin = 9;
constexpr int kSps30RxPin = 18;
constexpr int kSps30TxPin = 46;
constexpr uint32_t kSensorI2cClockHz = 50000;
constexpr uint32_t kSensorI2cTimeoutMs = 1000;
}  // namespace

bool useSIM = false;
bool simInitialized = false;

SIM7080 sim7080(kSimApn, kMqttHost, kMqttPort);
CredentialManager credential_manager;
NetworkServer server(credential_manager);
WiFiManager network(credential_manager);
TimeService timeService;
PayloadPublisher payloadPublisher(credential_manager, network, sim7080, useSIM);

TwoWire WireSensors = TwoWire(1);
HardwareSerial SerialSPS30(2);

SPS30 sps30;
SHT41Sensor sht41;
Handler handler;

RGBLight rgbLight(14, 13, 12);
bool rgbLightReady = false;

SensorReadoutManager sensorReadout(sps30, sht41, payloadPublisher, timeService,
                                   sim7080, useSIM, SerialSPS30, WireSensors,
                                   kSps30RxPin, kSps30TxPin);

void debugWarnIndicator() {
    if (rgbLightReady)
        rgbLight.warningBlink();
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

    timeService.syncFromSIM(sim7080);
    return true;
}

void setupNetworkTransport() {
    if (credential_manager.ValidateWifiCredentials()) {
        network.Connect();

        if (network.isConnected()) {
            useSIM = false;
            timeService.initializeNtpTime();
            payloadPublisher.initializeWifiMqttClient();
        } else {
            DEBUG_WARN("WiFi unavailable, falling back to SIM7080");
            switchToSIMNetwork();
        }
    } else {
        DEBUG_WARN("No WiFi credentials configured, using SIM7080");
        switchToSIMNetwork();
    }
}

void setup() {
    Serial.begin(115200);
    timeService.configureLocalTimezone();
    Serial.println();
    Serial.println("====================================");
    Serial.println("Starting GreenMile Air Quality Sensor");

    DEBUG_TRACE_SECTION("Setup");

    handler.setupRGB(rgbLight);
    rgbLightReady = true;
    handler.rgbInitialization(rgbLight);

    handler.setupCredentialManager(credential_manager, server, rgbLight);
    payloadPublisher.beginTopics();

    DEBUG_SECTION("Sensors");
    handler.setupSensorI2CBus(WireSensors, kSensorSdaPin, kSensorSclPin,
                              kSensorI2cClockHz, kSensorI2cTimeoutMs);

    // Start SPS30 before networking so its warmup overlaps WiFi/SIM setup.
    DEBUG_TRACE_KV("SPS30 UART ESP32 RX pin", kSps30RxPin);
    DEBUG_TRACE_KV("SPS30 UART ESP32 TX pin", kSps30TxPin);
    handler.setupSPS30(sps30, SerialSPS30, kSps30RxPin, kSps30TxPin, rgbLight);

    handler.setupSHT41(sht41, WireSensors, rgbLight);
    setupNetworkTransport();
    handler.disableRGB(rgbLight);
}

void loop() {
    static bool readPublishSectionLogged = false;
    if (!readPublishSectionLogged) {
        DEBUG_SECTION("Read And Publish");
        readPublishSectionLogged = true;
    }

    sensorReadout.handle(credential_manager.GetDeviceID());

    if (sensorReadout.allSent()) {
        DEBUG_OK("All data sent successfully, entering deep sleep");
        handler.enterDeepSleep(sps30, WireSensors, sim7080, kSensorSdaPin,
                               kSensorSclPin, useSIM);
    }

    wait(10);
}
