#pragma once

#include <Preferences.h>

// Persists Wi‑Fi and backend credentials using NVS Preferences.
// Provides simple validation and accessors for stored values.
class CredentialManager
{
public:
    CredentialManager() = default;

    // Validates that all required fields are non-empty.
    bool ValidateCredentials();

    // Stores credentials to NVS.
    void SaveCredentials(const String &ssid, const String &password, const String &device_id, const String &device_key);

    // Loads credentials from NVS into memory.
    void LoadCredentials();

    // Clears stored credentials from NVS.
    void ClearCredentials();

    String GetSSID() const { return ssid_; }
    String GetPassword() const { return password_; }
    String GetDeviceID() const { return device_id_; }
    String GetDeviceKey() const { return device_key_; }

private:
    String ssid_;
    String password_;
    String device_id_;
    String device_key_;
};
