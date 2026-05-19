#include "NetworkServer.h"
#include "DEBUG.h"


#include "HelpMethod.h"

// Starts a captive-like AP with a simple form to configure credentials.
void NetworkServer::StartAP()
{
    DEBUG_SECTION("AP Mode Start");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP_Config", "test1234");
    wait(500);

    DEBUG_INFO("AP started. Connect to 192.168.4.1");

    // Serve configuration form
    server_.on("/", HTTP_GET, [this]()
               { server_.send(200, "text/html", index_html); }); // index_html from html.h

    // Receive and persist credentials
    server_.on("/save", HTTP_POST, [this]()
               { handleSave(); });

    server_.begin();
}

// Must be called in the main loop to process HTTP requests.
void NetworkServer::HandleRequests()
{
    server_.handleClient();
}

// === AP mode ===
// Parses credentials from POST /save, stores them, and restarts the device.
void NetworkServer::handleSave()
{
    String ssid = server_.arg("ssid");
    String pass = server_.arg("pass");
    String device_id = server_.arg("device_id");
    String device_key = server_.arg("device_key");

    if (device_id.isEmpty() || device_key.isEmpty())
    {
        DEBUG_FAIL("Missing field in /save");
        server_.send(400, "text/plain", "Device ID and Device Key are required");
        return;
    }

    credential_manager_.SaveCredentials(ssid, pass, device_id, device_key);
    DEBUG_OK("Credentials saved. Rebooting...");
    server_.send(200, "text/plain", "Credentials saved. Rebooting...");
    wait(1000);
    ESP.restart();
}
