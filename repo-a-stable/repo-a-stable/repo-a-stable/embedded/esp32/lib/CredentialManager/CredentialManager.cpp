#include "CredentialManager.h"
#include "DEBUG.h"

// Checks that all credential fields are present.
bool CredentialManager::ValidateCredentials()
{
    DEBUG_SECTION("ValidateCredentials");

    DEBUG_LOG_LN("SSID: " + ssid_);

    // Uncommenting the following line will expose sensitive information in logs.
    // DEBUG_LOG_LN("Password: " + password_);
    // DEBUG_LOG_LN("Device ID: " + device_id_);
    // DEBUG_LOG_LN("Device Key: " + device_key_);

    if (ssid_.isEmpty() || password_.isEmpty() || device_id_.isEmpty() || device_key_.isEmpty())
    {
        DEBUG_FAIL("Invalid credentials");
        return false;
    }
    DEBUG_OK("Credentials are valid");
    return true;
}

// Loads credentials from the Preferences storage (read-only session).
void CredentialManager::LoadCredentials()
{
    Preferences prefs;
    prefs.begin("wifi", true);
    ssid_ = prefs.getString("ssid", "");
    password_ = prefs.getString("pass", "");
    device_id_ = prefs.getString("device_id", "");
    device_key_ = prefs.getString("device_key", "");
    prefs.end();
}

// Saves credentials to the Preferences storage (write session).
void CredentialManager::SaveCredentials(const String &ssid, const String &password, const String &device_id, const String &device_key)
{
    Preferences prefs;
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", password);
    prefs.putString("device_id", device_id);
    prefs.putString("device_key", device_key);
    prefs.end();

    DEBUG_OK("Credentials saved to NVS");
    DEBUG_LOG_LN("SSID: " + ssid);

    // Uncommenting the following line will expose sensitive information in logs.
    // DEBUG_LOG_LN("Password: " + password);
    // DEBUG_LOG_LN("Device ID: " + device_id);
    // DEBUG_LOG_LN("Device Key: " + device_key);
}

// Erases stored credentials and clears cached fields.
void CredentialManager::ClearCredentials()
{
    Preferences prefs;
    prefs.begin("wifi", false);
    prefs.clear();
    prefs.end();
    ssid_ = "";
    password_ = "";
    device_id_ = "";
    device_key_ = "";
}