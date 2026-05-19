#pragma once

#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "CredentialManager.h"
#include "DEBUG.h"

// WiFi connection manager for ESP32 devices.
//
// This class provides a simplified interface for WiFi operations on ESP32,
// including connection management, disconnection, and automatic reconnection.
class WiFiManager
{
public:
    WiFiManager(CredentialManager &credential_manager)
        : credential_manager_(credential_manager) {}

    // Establishes connection to the configured WiFi network.
    // [TODO]: Consider returning bool to signal connect success instead of relying on logs.
    void Connect();

    bool isConnected()
    {
        return WiFi.isConnected();
    }

    // Disconnects from WiFi network and powers down WiFi module.
    void Disconnect();

    // Attempts to reconnect to the configured WiFi network.
    // [TODO]: Accept a cancel flag or max attempts to prevent unbounded loops in some contexts.
    void Reconnect(uint64_t reconnectInterval = 10000);

private:
    CredentialManager &credential_manager_;

    const int connection_timeout_ =
        10000; // 10 seconds timeout for connection attempts
};
