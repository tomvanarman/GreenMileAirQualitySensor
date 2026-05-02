#include "WifiManager.h"

#include <Preferences.h>

#include "DEBUG.h"

// Attempts a blocking Wi‑Fi connection using stored credentials.
// Returns immediately if credentials are missing. Times out after
// connection_timeout_. [TODO]: Return a bool to indicate success/failure to
// callers instead of logging-only.
void WiFiManager::Connect() {
  DEBUG_SECTION("WiFi Connect");
  if (credential_manager_.GetSSID().isEmpty() ||
      credential_manager_.GetPassword().isEmpty()) {
    DEBUG_WARN("No WiFi credentials found!");
    return;
  }

  DEBUG_KV("SSID", credential_manager_.GetSSID());
  WiFi.mode(WIFI_STA);
  delay(100);
  WiFi.begin(credential_manager_.GetSSID().c_str(),
             credential_manager_.GetPassword().c_str());
  DEBUG_INFO("Connecting to WiFi...");

  int64_t start = millis();  // linter does not allow unsigned long
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > connection_timeout_) {
      DEBUG_FAIL("Connection timeout! Invalid credentials?");
      WiFi.disconnect(true);
      return;
    }

    delay(500);
  }

  DEBUG_OK("Connected to WiFi");
  DEBUG_KV("IP", WiFi.localIP().toString());
}

// Disconnects from AP and powers down radio.
void WiFiManager::Disconnect() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  DEBUG_INFO("Disconnected from WiFi");
}

// Reconnection loop that retries at a fixed interval until connected.
// [TODO]: Use exponential backoff with jitter to reduce contention and power
// use. [TODO]: Allow a cancellation condition (max attempts or external flag)
// to avoid infinite loops.
void WiFiManager::Reconnect(uint64_t reconnectInterval) {
  DEBUG_SECTION("WiFi Reconnect");
  uint64_t reconnect_start_time = millis();

  DEBUG_INFO(String("Retry every ") + String(reconnectInterval / 1000) + "s");

  // Keep trying to reconnect until successful
  while (!isConnected()) {
    uint64_t current_time = millis();

    // Check if enough time has passed since last reconnection attempt
    if (current_time - reconnect_start_time >= reconnectInterval) {
      DEBUG_BLOCK("Reconnection attempt");
      DEBUG_INFO("Attempting to reconnect...");

      // Attempt to reconnect to WiFi
      Connect();

      // Reset the timer for next reconnection attempt
      reconnect_start_time = millis();
    }

    delay(100);
    // [TODO]: Call yield() in tight loops to keep the watchdog happy on some
    // cores.
  }
  DEBUG_OK("Reconnected to WiFi");
}
