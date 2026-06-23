#include "PayloadPublisher.h"

#include <cstring>

#include "DEBUG.h"

namespace {
constexpr const char* kMqttUri = "mqtt://greenmile.tapp.city";
constexpr const char* kMqttHost = "greenmile.tapp.city";
constexpr uint32_t kMqttPort = 1883;
constexpr int kPayloadSendAttempts = 2;
constexpr const char* kSps30Path = "/api/data/sps30";
constexpr const char* kSht41Path = "/api/data/sht41";
constexpr const char* kBatteryPath = "/api/data/battery";
}

PayloadPublisher::PayloadPublisher(CredentialManager& credentialManager,
                                   WiFiManager& network, SIM7080& sim7080,
                                   bool& useSIM)
    : credentialManager_(credentialManager),
      network_(network),
      sim7080_(sim7080),
      useSIM_(useSIM) {}

void PayloadPublisher::beginTopics() {
    snprintf(sps30Topic_, sizeof(sps30Topic_), "greenmile/%s/sps30/data",
             credentialManager_.GetDeviceID().c_str());
    snprintf(sht41Topic_, sizeof(sht41Topic_), "greenmile/%s/sht41/data",
             credentialManager_.GetDeviceID().c_str());
    snprintf(batteryTopic_, sizeof(batteryTopic_), "greenmile/%s/battery/data",
             credentialManager_.GetDeviceID().c_str());
}

bool PayloadPublisher::initializeWifiMqttClient() {
    if (client_ != nullptr)
        return true;

    esp_mqtt_client_config_t mqttCfg = {};
    mqttCfg.host = kMqttHost;
    mqttCfg.uri = kMqttUri;
    mqttCfg.port = kMqttPort;

    client_ = esp_mqtt_client_init(&mqttCfg);
    if (client_ == nullptr) {
        DEBUG_WARN("Failed to initialize MQTT client");
        return false;
    }

    esp_mqtt_client_register_event(client_, MQTT_EVENT_CONNECTED,
                                   mqttEventHandler, nullptr);
    esp_mqtt_client_register_event(client_, MQTT_EVENT_DATA, mqttEventHandler,
                                   nullptr);
    esp_mqtt_client_register_event(client_, MQTT_EVENT_DISCONNECTED,
                                   mqttEventHandler, nullptr);

    esp_err_t startResult = esp_mqtt_client_start(client_);
    if (startResult != ESP_OK) {
        DEBUG_WARN("Failed to start MQTT client");
        client_ = nullptr;
        return false;
    }

    return true;
}

bool PayloadPublisher::sendPayload(const char* path, const String& payload) {
    const char* topic = getMqttTopicForPath(path);
    if (topic == nullptr) {
        DEBUG_WARN("No MQTT topic configured for payload path");
        return false;
    }

    for (int attempt = 1; attempt <= kPayloadSendAttempts; attempt++) {
        DEBUG_TRACE_KV("Payload send attempt", attempt);

        if (useSIM_) {
            if (!sim7080_.ensureConnected()) {
                DEBUG_WARN("Failed to connect to SIM network");
            } else if (sim7080_.mqttPublish(
                           credentialManager_.GetDeviceID().c_str(), topic,
                           payload.c_str())) {
                return true;
            }
        } else {
            if (!network_.isConnected()) {
                DEBUG_WARN("WiFi disconnected, reconnecting on WiFi");
                network_.Connect();
            }

            if (network_.isConnected()) {
                if (client_ == nullptr && !initializeWifiMqttClient()) {
                    DEBUG_WARN("MQTT client is not ready");
                } else if (esp_mqtt_client_publish(
                               client_, topic, payload.c_str(), 0, 1, 0) >= 0) {
                    return true;
                } else {
                    DEBUG_WARN("MQTT publish failed");
                }
            }
        }

        yield();
    }

    return false;
}

const char* PayloadPublisher::sps30Path() {
    return kSps30Path;
}

const char* PayloadPublisher::sht41Path() {
    return kSht41Path;
}

const char* PayloadPublisher::batteryPath() {
    return kBatteryPath;
}

const char* PayloadPublisher::getMqttTopicForPath(const char* path) const {
    if (strcmp(path, kSps30Path) == 0)
        return sps30Topic_;
    if (strcmp(path, kSht41Path) == 0)
        return sht41Topic_;
    if (strcmp(path, kBatteryPath) == 0)
        return batteryTopic_;
    return nullptr;
}

void PayloadPublisher::mqttEventHandler(void* handlerArgs,
                                        esp_event_base_t base,
                                        int32_t eventId, void* eventData) {
    (void)handlerArgs;
    (void)base;
    esp_mqtt_event_handle_t event =
        static_cast<esp_mqtt_event_handle_t>(eventData);

    switch (static_cast<esp_mqtt_event_id_t>(eventId)) {
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
