// External libraries
#include <Arduino.h>
#include <Wire.h>

// Custom structured debug logging
#include "DEBUG.h"

// JSON builder utility
#include <JsonBuilder.h>

// Manager Libraries
#include "NetworkServer.h"
#include "CredentialManager.h"
#include "MqttClient.h"
#include "WifiManager.h"

// Sensor Libraries
#include "PushButton.h"
#include "SHT41Sensor.h"

// SIM7080 Modem Library
#include "SIM7080.h"

// These crednentials are now provided via the NetworkServer AP mode and stored in NVS
// constexpr char kSsid[] = "test";
// constexpr char kPassword[] = "test1234";
// constexpr char kMqttServer[] = "192.168.137.1";
// constexpr int kMqttPort = 1883;

// MQTT topics
constexpr char kSht41Topic[] = "data/sht41";

// NTP server configuration
const char *ntpServer1 = "149.143.87.22"; // pool.ntp.org
const char *ntpServer2 = "82.65.248.56";  // europe.pool.ntp.org
const long gmtOffset_sec = 3600;          // GMT+1
const int daylightOffset_sec = 0;

// Timing constants (in milliseconds)
constexpr unsigned long kReconnectInterval = 6000;
constexpr unsigned long kDataTransmissionInterval = 5000;

// Global objects
CredentialManager credential_manager;

NetworkServer server(credential_manager);
WiFiManager network(credential_manager);
MqttClient mqtt_client(credential_manager, &network);

SHT41Sensor sht41;               // Uses default pins 8,9 for ESP32-S3
PushButton credential_reset(14); // Push button connected to GPIO pin 14

SIM7080 modem;

// Function declarations
void HandleButtonLogic();
void HandleSHT41Logic();
void InitializeTime();
uint64_t GetCurrentTimestampMs();

void setup()
{
  Serial.begin(115200);

  // Wait for serial port to be ready
  while (!Serial)
    ;

  DEBUG_SECTION("Setup");
  DEBUG_KV("MAC Address", WiFi.macAddress());

  credential_manager.LoadCredentials();

  if (!credential_manager.ValidateCredentials())
  {
    DEBUG_WARN("Invalid or missing credentials, starting AP for configuration...");
    server.StartAP();

    // Wait indefinitely in AP mode until configured
    while (true)
    {
      server.HandleRequests();
      delay(10);
    }
  }

  // // Initialize I2C communication
  // Wire.begin();

  // // Wait for sensor to be ready
  // if (!sht41.begin())
  // {
  //   DEBUG_WARN("Failed to find SHT41 sensor");
  //   while (1)
  //     delay(10);
  // }

  // Establish initial WiFi connection
  network.Connect();

  if (!network.isConnected())
  {
    network.Reconnect(kReconnectInterval);
  }

  // Initialize NTP time synchronization
  InitializeTime();

  // Establish initial MQTT connection after WiFi is ready
  mqtt_client.Connect();
}

void loop()
{
  while (network.isConnected() && mqtt_client.IsConnected())
  {
    HandleButtonLogic();
    HandleSHT41Logic();

    // Maintain MQTT connection and process incoming messages
    mqtt_client.Loop();
  }

  if (!network.isConnected())
  {
    DEBUG_WARN("WiFi disconnected");
    network.Reconnect(kReconnectInterval);

    // After WiFi reconnects, MQTT will need to reconnect too
    mqtt_client.Connect();
  }

  if (!mqtt_client.IsConnected())
  {
    DEBUG_WARN("MQTT disconnected");
    mqtt_client.Reconnect();
  }
}

void HandleButtonLogic()
{
  if (credential_reset.IsPressed())
  {
    DEBUG_SECTION("Credential Reset");
    DEBUG_WARN("Reset button pressed, clearing stored credentials...");
    credential_manager.ClearCredentials();
    DEBUG_INFO("Credentials cleared, restarting device...");
    ESP.restart();
  }
}

// Read out the sht41 sensor
void HandleSHT41Logic()
{
  unsigned long now = millis();
  bool can_update = now - sht41.last_update >= kDataTransmissionInterval;
  // SHT41Data shtData = sht41.readData(75);

  if (can_update)
  {
    // if (shtData.available)
    // {
    //   sht41.last_update = now;

    //   // Pretty print for debug
    //   String prettyData = JsonBuilder::BuildJson([&](JsonDocument &doc)
    //                                              {
    //         doc["temperature"] = shtData.temperature;
    //         doc["humidity"] = shtData.humidity;
    //         doc["time_unix"] = GetCurrentTimestampMs() / 1000; }, true);
    //   DEBUG_BLOCK("Payload");
    //   DEBUG_INFO(prettyData);

    //   // Compact version for actual POST
    //   String data = JsonBuilder::BuildJson([&](JsonDocument &doc)
    //                                        {
    //         doc["temperature"] = shtData.temperature;
    //         doc["humidity"] = shtData.humidity;
    //         doc["time_unix"] = GetCurrentTimestampMs() / 1000; });

    //   mqtt_client.Publish(kSht41Topic, data);
    // }
    // else
    // {
    //   DEBUG_WARN("Failed to read from sht41 sensor");
    // }

    // Dummy data for testing without sensor
    sht41.last_update = now;

    // Pretty print for debug
    String prettyData = JsonBuilder::BuildJson([&](JsonDocument &doc)
                                               {
          doc["temperature"] = 25.0;
          doc["humidity"] = 50.0;
          doc["time_unix"] = GetCurrentTimestampMs() / 1000; }, true);
    DEBUG_BLOCK("Payload");
    DEBUG_INFO(prettyData);

    // Compact version for actual POST
    String data = JsonBuilder::BuildJson([&](JsonDocument &doc)
                                         {
          doc["temperature"] = 25.0;
          doc["humidity"] = 50.0;
          doc["time_unix"] = GetCurrentTimestampMs() / 1000; });

    mqtt_client.Publish(kSht41Topic, data);
  }
}

void InitializeTime()
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
}

uint64_t GetCurrentTimestampMs()
{
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<uint64_t>(tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
}
