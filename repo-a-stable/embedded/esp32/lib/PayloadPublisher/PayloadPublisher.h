#pragma once

#include <Arduino.h>
#include <esp_event.h>
#include <mqtt_client.h>

#include "../CredentialManager/CredentialManager.h"
#include "../SIM7080/SIM7080.h"
#include "../WifiManager/WifiManager.h"

class PayloadPublisher {
 public:
  PayloadPublisher(CredentialManager& credentialManager,  // NOLINT(runtime/references)
                   WiFiManager& network,  // NOLINT(runtime/references)
                   SIM7080& sim7080,  // NOLINT(runtime/references)
                   bool& useSIM);  // NOLINT(runtime/references)

  void beginTopics();
  bool initializeWifiMqttClient();
  bool sendPayload(const char* path, const String& payload);

  static const char* sps30Path();
  static const char* sht41Path();
  static const char* batteryPath();

 private:
  const char* getMqttTopicForPath(const char* path) const;
  static void mqttEventHandler(void* handlerArgs, esp_event_base_t base,
                               int32_t eventId, void* eventData);

  CredentialManager& credentialManager_;
  WiFiManager& network_;
  SIM7080& sim7080_;
  bool& useSIM_;
  esp_mqtt_client_handle_t client_ = nullptr;
  char sps30Topic_[64] = {};
  char sht41Topic_[64] = {};
  char batteryTopic_[64] = {};
};
