// External libraries
#include <Arduino.h>
#include <Handler.h>
#include <Wire.h>

#include <algorithm>
#include <deque>

// Custom structured debug logging
#include "DEBUG.h"

// Utility Libraries
#include <JsonBuilder.h>

#include "ColorMap.h"
#include "HelpMethod.h"

// Sensor Libraries
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

bool useSIM = false;

// Server configuration for HTTPS POST
const char *host = "greenmile.tapp.city";
const char *url = "https://greenmile.tapp.city";
const char *SPS30path = "/api/data/sps30";
const char *SHT41path = "/api/data/sht41";
const char *BatteryPath = "/api/data/battery";

// Timing constants
constexpr unsigned long kReconnectInterval = 6000;
constexpr unsigned long kDataTransmissionInterval = 10000;
constexpr unsigned long kDataMeasurementInterval = 2000;

// Global objects
SIM7080 sim7080("iot.1nce.net"); // APN for 1NCE IoT SIM cards

CredentialManager credential_manager;
NetworkServer server(credential_manager);
WiFiManager network(credential_manager);
HttpManager httpManager;

TwoWire WireSensors = TwoWire(1);

SPS30 sps30;
SHT41Sensor sht41;
Handler handler;

LEDStrip strip;
SegmentDisplay segmentDisplay(11, 12, 10); // Data, CLK, CS pins

// For debugging purposes
float maxPM = 25.0f;
GradientStop stops[] = {
    {0.0f / maxPM, 0, 255, 0},    // green
    {5.0f / maxPM, 255, 255, 0},  // yellow
    {10.0f / maxPM, 255, 165, 0}, // orange
    {15.0f / maxPM, 255, 0, 0},   // red
    {25.0f / maxPM, 128, 0, 0},   // maroon
};

ColorMap colorMap(maxPM, stops);

// NTP server configuration
const char *ntpServer1 = "149.143.87.22"; // pool.ntp.org
const char *ntpServer2 = "82.65.248.56";  // europe.pool.ntp.org
const long gmtOffset_sec = 3600;          // GMT+1
const int daylightOffset_sec = 0;

// Function declarations
bool HandleSPS30Logic();
bool HandleSHT41Logic();
bool HandleBatteryLogic();
bool sendPayload(const char *path, const String &payload);
void initializeTime();
uint64_t getCurrentTimestampMs();
int determineBatteryLevel();

void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println("====================================");
  Serial.println("Starting GreenMile Air Quality Sensor");

  wait(5000); // Give some time to open the serial monitor after reset

  DEBUG_SECTION("Setup");

  strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BREATHING);

  if (useSIM) {
    // ============================================================================
    // Setup SIM7080
    // ============================================================================
    handler.setupSim7080(sim7080, strip);
  } else {
    handler.setupCredentialManager(credential_manager, server, strip,
                                   segmentDisplay);

    //============================================================================
    // Setup WiFi
    //============================================================================
    handler.setupWifi(network, server, strip, segmentDisplay);

    // Initialize NTP time synchronization
    initializeTime();
  }

  strip.stopLoading();
  strip.clear();

  // Initialize I2C communication
  WireSensors.begin(8, 9);      // SDA, SCL
  WireSensors.setClock(100000); // lock to 100 kHz for both SPS30 and SHT41
  WireSensors.setTimeOut(100);  // a bit more headroom for long reads

  //============================================================================
  // Setup SPS30
  //============================================================================
  handler.setupSPS30(sps30, WireSensors, strip);

  //============================================================================
  // Setup SHT41
  //============================================================================
  handler.setupSHT41(sht41, WireSensors, strip);

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

  bool spsSent = HandleSPS30Logic();
  bool shtSent = HandleSHT41Logic();
  // bool batterySent = HandleBatteryLogic();

  if (spsSent && shtSent) {
    DEBUG_OK("All data sent successfully, entering deep sleep");
    wait(500);
    handler.enterDeepSleep(strip, segmentDisplay, useSIM);
  }

  wait(250);
}

bool HandleSPS30Logic() {
  unsigned long now = millis();
  bool can_measure = now - sps30.last_measurement >= kDataMeasurementInterval;
  bool can_update = now - sps30.last_update >= kDataTransmissionInterval;

  if (!can_measure)
    return false;

  SPS30_measurement spsData = sps30.readData();

  if (!spsData.available) {
    DEBUG_WARN("Failed to read from sps30 sensor");

    if (!strip.isLoading())
      strip.startLoading(CRGB::DeepSkyBlue,
                         LEDStrip::_loadingModeType::BLINKING);

    return false;
  }

  if (strip.isLoading())
    strip.stopLoading();

  sps30.last_measurement = now;

  // Update LED strip based on PM2.5 value
  Color c = colorMap.DataToColor(spsData.mc_2p0);
  strip.toColor(CRGB(c.r, c.g, c.b), 50);

  if (!can_update)
    return false;

  sps30.last_update = now;

  // Pretty print for debug
  String prettyData = JsonBuilder::BuildJson(
      [&](JsonDocument &doc) {
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
  String data = JsonBuilder::BuildJson([&](JsonDocument &doc) {
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
  unsigned long now = millis();
  bool can_measure = now - sht41.last_measurement >= kDataMeasurementInterval;
  bool can_update = now - sht41.last_update >= kDataTransmissionInterval;

  if (!can_measure)
    return false;

  SHT41Data shtData = sht41.readData(10);

  if (!shtData.available) {
    DEBUG_WARN("Failed to read from SHT41 sensor");
    segmentDisplay.setError();
    return false;
  }

  segmentDisplay.setTemp(shtData.temperature);
  segmentDisplay.setHum(shtData.humidity);

  sht41.last_measurement = now;

  if (!can_update)
    return false;

  sht41.last_update = now;

  // Pretty print for debug
  String prettyData = JsonBuilder::BuildJson(
      [&](JsonDocument &doc) {
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
  String data = JsonBuilder::BuildJson([&](JsonDocument &doc) {
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
  unsigned long now = millis();
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
      [&](JsonDocument &doc) {
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
  String data = JsonBuilder::BuildJson([&](JsonDocument &doc) {
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

bool sendPayload(const char *path, const String &payload) {
  auto [signatureOk, signature] = httpManager.signBody(
      payload.c_str(), credential_manager.GetDeviceKey().c_str());

  if (!signatureOk) {
    DEBUG_WARN("Failed to sign request body");
    return false;
  }

  if (useSIM) {
    // TODO add signature logic to SIM7080 if needed

    if (!sim7080.ensureConnected()) {
      DEBUG_WARN("Failed to connect to network");
      strip.stopLoading();
      strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);
      return false;
    }

    if (sim7080.httpPost(credential_manager.GetDeviceID().c_str(),
                         signature.c_str(), host, path, url, payload.c_str())) {
      return true;
    }
  } else {
    if (httpManager.post(String(credential_manager.GetDeviceID()),
                         signature.c_str(), String(url), String(path),
                         payload)) {
      return true;
    }
  }

  return false;
}

void initializeTime() {
  DEBUG_SECTION("NTP Sync");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  DEBUG_INFO("Waiting for NTP time sync...");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo, 10000)) {
    DEBUG_WARN("Failed to obtain time, retrying...");
    wait(2000);
  }
  DEBUG_OK("Time synchronized");
  DEBUG_KV("Current time", String(asctime(&timeinfo)));
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