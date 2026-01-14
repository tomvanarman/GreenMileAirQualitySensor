#pragma once

#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>

#include "WifiManager.h"

// MQTT client wrapper class for ESP32 devices.
//
// This class provides a simplified interface for MQTT operations on ESP32,
// including connection management, message publishing, and automatic
// reconnection. It depends on WiFi connectivity and integrates with the
// WifiManager for network status checking.
class MqttClient {
 public:
  // Constructs an MQTT client with the specified broker configuration.
  MqttClient(CredentialManager &credential_manager, WiFiManager *wifi_manager) 
      : credential_manager_(credential_manager),
        wifi_manager_(wifi_manager),
        esp_client_(),
        client_(esp_client_) {}

  // Establishes connection to the MQTT broker.
  void Connect();

  // Closes the MQTT connection gracefully.
  void Disconnect();

  // Attempts to reconnect to the MQTT broker.
  void Reconnect();

  // Publishes a message to the specified MQTT topic.
  void Publish(const char *topic, const String &payload);

  // Returns the current MQTT connection status.
  bool IsConnected();

  // Processes incoming MQTT messages and maintains connection.
  void Loop();

 private:
  CredentialManager &credential_manager_;
  WiFiManager *wifi_manager_;
  WiFiClientSecure esp_client_;
  PubSubClient client_;
};
