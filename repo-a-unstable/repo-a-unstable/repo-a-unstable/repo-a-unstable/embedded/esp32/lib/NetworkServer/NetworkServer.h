#pragma once

#include <WiFi.h>
#include <WebServer.h>

#include "DEBUG.h"
#include "CredentialManager.h"
#include "html.h"
// Lightweight HTTP server wrapper for ESP32 that exposes a minimal
// configuration portal (AP mode) and a small set of client-mode endpoints.
//
// Intent: keep transport/parsing here and leave hardware/business logic to
// higher layers (main.cpp). This keeps responsibilities clear and
// simplifies testing.
class NetworkServer
{
public:
    NetworkServer(CredentialManager &credential_manager) : server_(80),
                                                           credential_manager_(credential_manager) {}

    // Starts an access point with a basic HTML form to capture Wi‑Fi and API credentials.
    // Route(s):
    //   GET  /       -> HTML form
    //   POST /save   -> Persist credentials and reboot
    void StartAP();

    // Must be called frequently from loop() to process pending client requests.
    void HandleRequests();

private:
    WebServer server_;
    CredentialManager &credential_manager_;

    // Renders the minimal HTML configuration form.
    String getHtmlForm();

    // Handles POST /save to store credentials (restarts device on success).
    void handleSave();
};
