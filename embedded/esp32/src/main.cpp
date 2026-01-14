// External libraries
#include <Arduino.h>
#include <Wire.h>
#include <deque>
#include <algorithm>

// Custom structured debug logging
#include "DEBUG.h"

// Utility Libraries
#include <JsonBuilder.h>
#include "ColorMap.h"

// Sensor Libraries
#include "SPS30.h"
#include "SHT41Sensor.h"

// Actuator Libraries
#include "LEDStrip.h"
#include "SegmentDisplay.h"

// SIM7080 Modem Library
#include "SIM7080.h"

// Network Libraries
#include "NetworkServer.h"
#include "CredentialManager.h"
#include "WifiManager.h"
#include "HttpManager.h"

bool useSIM = true;

// Server configuration for HTTPS POST
const char *host = "greenmile.tapp.city";
const char *url = "https://greenmile.tapp.city";
const char *SPS30path = "/api/data/sps30";
const char *SHT41path = "/api/data/sht41";
const char *BatteryPath = "/api/data/battery";

// Timing constants (in milliseconds)
constexpr unsigned long kReconnectInterval = 6000;
constexpr unsigned long kDataTransmissionInterval = 30000;
constexpr unsigned long kDataMeasurementInterval = 10000;

// Global objects
SIM7080 sim7080("iot.1nce.net"); // APN for 1NCE IoT SIM cards

CredentialManager credential_manager;
NetworkServer server(credential_manager);
WiFiManager network(credential_manager);
HttpManager httpManager;

TwoWire WireSensors = TwoWire(1);

SPS30 sps30;
SHT41Sensor sht41;

LEDStrip strip;
SegmentDisplay segmentDisplay(11, 12, 10); // Data, CLK, CS pins

float maxPM = 250.0f;
GradientStop stops[] = {
    {0.0f / maxPM, 0, 255, 0},      // green
    {50.0f / maxPM, 255, 255, 0},   // yellow
    {100.0f / maxPM, 255, 165, 0},  // orange
    {150.0f / maxPM, 255, 0, 0},    // red
    {250.0f / maxPM, 128, 0, 0},  // maroon
};

// // For debugging purposes
// float maxPM = 25.0f;
// GradientStop stops[] = {
//     {0.0f / maxPM, 0, 255, 0},    // green
//     {5.0f / maxPM, 255, 255, 0},  // yellow
//     {10.0f / maxPM, 255, 165, 0}, // orange
//     {15.0f / maxPM, 255, 0, 0},   // red
//     {25.0f / maxPM, 128, 0, 0},   // maroon
// };

ColorMap colorMap(maxPM, stops);

std::deque<String> postQueue;
const size_t kMaxQueueSize = 50;

// NTP server configuration
const char *ntpServer1 = "149.143.87.22"; // pool.ntp.org
const char *ntpServer2 = "82.65.248.56";  // europe.pool.ntp.org
const long gmtOffset_sec = 3600;          // GMT+1
const int daylightOffset_sec = 0;

// Function declarations
void HandleSPS30Logic();
void HandleSHT41Logic();
void HandleBatteryLogic();
bool trySendQueue(const char *path);
bool initializeTime();
uint64_t getCurrentTimestampMs();
int determineBatteryLevel();

void setup()
{
  Serial.begin(115200);

  // Wait for serial port to be ready
  while (!Serial)
    ;

  delay(5000);

  DEBUG_SECTION("Setup");

  // ============================================================================
  // Setup credentials
  // ============================================================================
  strip.startLoading(CRGB::DarkRed, LEDStrip::_loadingModeType::BREATHING);
  credential_manager.LoadCredentials();

  if (!credential_manager.ValidateCredentials())
  {
    DEBUG_WARN("Invalid or missing credentials, starting AP for configuration...");
    server.StartAP();

    strip.stopLoading();
    strip.startLoading(CRGB::DarkRed, LEDStrip::_loadingModeType::BLINKING);

    segmentDisplay.start();
    segmentDisplay.setIPAddress("192.168.4.1");

    // Wait indefinitely in AP mode until configured
    while (true)
    {
      server.HandleRequests();
      delay(10);
    }
  }

  strip.stopLoading();
  strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BREATHING);

  if (useSIM)
  {
    // ============================================================================
    // Setup SIM7080
    // ============================================================================
    sim7080.initialize();
    sim7080.startModem();
    sim7080.setupNetwork();
    if (!sim7080.ensureConnected())
    {
      DEBUG_WARN("Failed to connect to network");
      strip.stopLoading();
      strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);
      while (1)
        sim7080.ensureConnected();
    }
    strip.stopLoading();
    strip.clear();
  }
  else
  {
    //============================================================================
    // Setup WiFi
    //============================================================================
    network.Connect();

    if (!network.isConnected())
    {
      DEBUG_WARN("Wrong credentials, starting AP for configuration...");
      server.StartAP();

      strip.stopLoading();
      strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);

      segmentDisplay.start();
      segmentDisplay.setIPAddress("192.168.4.1");

      // Wait indefinitely in AP mode until configured
      while (true)
      {
        server.HandleRequests();
        delay(10);
      }
    }

    //============================================================================
    // Setup time when using WiFi
    //============================================================================
    strip.stopLoading();
    strip.startLoading(CRGB::Goldenrod, LEDStrip::_loadingModeType::BREATHING);

    // Initialize NTP time synchronization
    if (!initializeTime())
    {
      DEBUG_WARN("Failed to setup time");

      strip.stopLoading();
      strip.startLoading(CRGB::Goldenrod, LEDStrip::_loadingModeType::BLINKING);
    }
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
  strip.startLoading(CRGB::DeepSkyBlue, LEDStrip::_loadingModeType::BREATHING);
  if (!sps30.begin(WireSensors))
  {
    DEBUG_WARN("Failed to find SPS30 sensor");
    strip.stopLoading();
    strip.startLoading(CRGB::DeepSkyBlue, LEDStrip::_loadingModeType::BLINKING);
    while (1)
      delay(10);
  }
  strip.stopLoading();

  //============================================================================
  // Setup SHT41
  //============================================================================
  strip.startLoading(CRGB::DarkBlue, LEDStrip::_loadingModeType::BREATHING);
  if (!sht41.begin(WireSensors))
  {
    DEBUG_WARN("Failed to find SHT41 sensor");
    strip.stopLoading();
    strip.startLoading(CRGB::DarkBlue, LEDStrip::_loadingModeType::BLINKING);
    while (1)
      delay(10);
  }
  strip.stopLoading();
  strip.clear();

  // Initialize LUT for color mapping
  colorMap.InitLUT();

  // Initialize segment display
  segmentDisplay.start();
  segmentDisplay.clearDisplay();
}

void loop()
{
  HandleSHT41Logic();
  HandleSPS30Logic();
  HandleBatteryLogic();
}

void HandleSPS30Logic()
{
  unsigned long now = millis();
  bool can_measure = now - sps30.last_measurement >= kDataMeasurementInterval;
  bool can_update = now - sps30.last_update >= kDataTransmissionInterval;

  if (!can_measure)
    return;

  SPS30_measurement spsData = sps30.readData();

  if (!spsData.available)
  {
    DEBUG_WARN("Failed to read from sps30 sensor");

    if (!strip.isLoading())
      strip.startLoading(CRGB::DeepSkyBlue, LEDStrip::_loadingModeType::BLINKING);

    return;
  }

  if (strip.isLoading())
    strip.stopLoading();

  sps30.last_measurement = now;

  // Update LED strip based on PM2.5 value
  Color c = colorMap.DataToColor(spsData.mc_2p0);
  strip.toColor(CRGB(c.r, c.g, c.b), 5000);

  if (!can_update)
    return;

  sps30.last_update = now;

  // Pretty print for debug
  String prettyData = JsonBuilder::BuildJson([&](JsonDocument &doc)
                                             {
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
            doc["time_unix"] = getCurrentTimestampMs() / 1000; }, true);

  DEBUG_BLOCK("Payload");
  DEBUG_INFO(prettyData);

  // Compact version for actual POST
  String data = JsonBuilder::BuildJson([&](JsonDocument &doc)
                                       {
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
            doc["time_unix"] = getCurrentTimestampMs() / 1000; });

  // Add to post queue
  postQueue.push_back(data);

  // Ensure queue does not exceed max size
  if (postQueue.size() > kMaxQueueSize)
  {
    postQueue.pop_front();
    DEBUG_WARN("Post queue full, dropping oldest data");
  }

  // Try to send queued data
  while (!postQueue.empty())
  {
    if (trySendQueue(SPS30path))
    {
      DEBUG_INFO("Successfully sent queued data");
      DEBUG_KV("Remaining queue size", postQueue.size());
    }
    else
    {
      DEBUG_WARN("Failed to send queued data, will retry later");
      DEBUG_KV("Current queue size", postQueue.size());
      break;
    }
  }
}

// Read out the sht41 sensor
void HandleSHT41Logic()
{
  unsigned long now = millis();
  bool can_measure = now - sht41.last_measurement >= kDataMeasurementInterval;
  bool can_update = now - sht41.last_update >= kDataTransmissionInterval;

  if (!can_measure)
    return;

  SHT41Data shtData = sht41.readData(10);

  if (!shtData.available)
  {
    DEBUG_WARN("Failed to read from SHT41 sensor");
    segmentDisplay.setError();
    return;
  }

  segmentDisplay.setTemp(shtData.temperature);
  segmentDisplay.setHum(shtData.humidity);

  sht41.last_measurement = now;

  if (!can_update)
    return;

  sht41.last_update = now;

  // Pretty print for debug
  String prettyData = JsonBuilder::BuildJson([&](JsonDocument &doc)
                                             {
          doc["temperature"] = shtData.temperature;
          doc["humidity"] = shtData.humidity;

          if (useSIM)
            doc["time_unix"] = sim7080.getCurrentTimestampMs() / 1000;
          else
            doc["time_unix"] = getCurrentTimestampMs() / 1000; }, true);

  DEBUG_BLOCK("Payload");
  DEBUG_INFO(prettyData);

  // Compact version for actual POST
  String data = JsonBuilder::BuildJson([&](JsonDocument &doc)
                                       {
          doc["temperature"] = shtData.temperature;
          doc["humidity"] = shtData.humidity;

          if (useSIM)
            doc["time_unix"] = sim7080.getCurrentTimestampMs() / 1000;
          else
            doc["time_unix"] = getCurrentTimestampMs() / 1000; });

  // Add to post queue
  postQueue.push_back(data);

  // Ensure queue does not exceed max size
  if (postQueue.size() > kMaxQueueSize)
  {
    postQueue.pop_front();
    DEBUG_WARN("Post queue full, dropping oldest data");
  }

  // Try to send queued data
  while (!postQueue.empty())
  {
    if (trySendQueue(SHT41path))
    {
      DEBUG_INFO("Successfully sent queued data");
      DEBUG_KV("Remaining queue size", postQueue.size());
    }
    else
    {
      DEBUG_WARN("Failed to send queued data, will retry later");
      DEBUG_KV("Current queue size", postQueue.size());
      break;
    }
  }
}

// Read out the batter
void HandleBatteryLogic()
{
  unsigned long now = millis();
  bool can_measure = now - sim7080.last_battery_measurement >= kDataMeasurementInterval;
  bool can_update = now - sim7080.last_battery_update >= kDataTransmissionInterval;

  if (!can_measure)
    return;

  int batteryLevel = determineBatteryLevel();

  sim7080.last_battery_measurement = now;

  if (!can_update)
    return;

  sim7080.last_battery_update = now;

  // Pretty print for debug
  String prettyData = JsonBuilder::BuildJson([&](JsonDocument &doc)
                                             {
          doc["level"] = batteryLevel;

          if (useSIM)
            doc["time_unix"] = sim7080.getCurrentTimestampMs() / 1000;
          else
            doc["time_unix"] = getCurrentTimestampMs() / 1000; }, true);

  DEBUG_BLOCK("Payload");
  DEBUG_INFO(prettyData);

  // Compact version for actual POST
  String data = JsonBuilder::BuildJson([&](JsonDocument &doc)
                                       {
          doc["level"] = batteryLevel;
          
          if (useSIM)
            doc["time_unix"] = sim7080.getCurrentTimestampMs() / 1000;
          else
            doc["time_unix"] = getCurrentTimestampMs() / 1000; });

  // Add to post queue
  postQueue.push_back(data);

  // Ensure queue does not exceed max size
  if (postQueue.size() > kMaxQueueSize)
  {
    postQueue.pop_front();
    DEBUG_WARN("Post queue full, dropping oldest data");
  }

  // Try to send queued data
  while (!postQueue.empty())
  {
    if (trySendQueue(SHT41path))
    {
      DEBUG_INFO("Successfully sent queued data");
      DEBUG_KV("Remaining queue size", postQueue.size());
    }
    else
    {
      DEBUG_WARN("Failed to send queued data, will retry later");
      DEBUG_KV("Current queue size", postQueue.size());
      break;
    }
  }
}

bool trySendQueue(const char *path)
{
  if (postQueue.empty())
    return true;

  // Always send the oldest entry
  const String &current = postQueue.front();

  httpManager.signBody(current.c_str(), credential_manager.GetDeviceKey().c_str());
  auto [signatureOk, signature] = httpManager.signBody(current.c_str(), credential_manager.GetDeviceKey().c_str());

  if (!signatureOk)
  {
    DEBUG_WARN("Failed to sign request body");
    return false;
  }

  if (useSIM)
  {
    // TODO add signature logic to SIM7080

    if (!sim7080.ensureConnected())
    {
      DEBUG_WARN("Failed to connect to network");
      strip.stopLoading();
      strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);

      while (!sim7080.ensureConnected())
        ;
    }

    if (sim7080.httpPost(credential_manager.GetDeviceID().c_str(),
                         signature.c_str(),
                         host,
                         path,
                         url,
                         current.c_str()))
    {
      postQueue.pop_front();
      return true;
    }
  }
  else
  {
    if (httpManager.post(String(credential_manager.GetDeviceID()),
                         signature.c_str(),
                         String(url),
                         String(path),
                         current))
    {
      postQueue.pop_front();
      return true;
    }
  }

  return false;
}

bool initializeTime()
{
  DEBUG_SECTION("NTP Sync");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  DEBUG_INFO("Waiting for NTP time sync...");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo, 10000))
  {
    DEBUG_WARN("Failed to obtain time, retrying...");
    delay(2000);
  }
  DEBUG_OK("Time synchronized");
  DEBUG_KV("Current time", String(asctime(&timeinfo)));

  delay(5000);

  return true;
}

uint64_t getCurrentTimestampMs()
{
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<uint64_t>(tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
}

int determineBatteryLevel()
{
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

  if (level < maxAllowedLevel)
  {
    level = maxAllowedLevel;
  }

  return level;
}
