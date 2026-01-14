#include "HttpManager.h"

/**
 * @brief Converts a hex character to its numeric value
 */
inline uint8_t hexCharToValue(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return 0;
}

/**
 * @brief Signs the given body using HMAC-SHA256 with the provided secret.
 * @returns a pair where the first element indicates success, and the second is
 * the resulting signature in hex format.
 */
std::pair<bool, std::string> HttpManager::signBody(const char* body, const char* secret) {
  const mbedtls_md_info_t* md_info =
      mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

  if (!md_info) {
    return {false, ""};
  }

  size_t hex_len = strlen(secret);

  // Hex string must have even length
  if (hex_len % 2 != 0) {
    return {false, ""};
  }
  size_t secret_len = hex_len / 2;
  uint8_t secret_bytes[256];
  if (secret_len > sizeof(secret_bytes)) {
    return {false, ""};
  }
  // Convert hex string to bytes
  for (size_t i = 0; i < secret_len; i++) {
    secret_bytes[i] = (hexCharToValue(secret[i * 2]) << 4) |
                      hexCharToValue(secret[i * 2 + 1]);
  }

  uint8_t hmac[32];

  int r = mbedtls_md_hmac(md_info, secret_bytes, secret_len,
                          reinterpret_cast<const unsigned char*>(body),
                          std::strlen(body), hmac);

  if (r != 0) return {false, ""};

  char hex_output[65];
  for (size_t i = 0; i < 32; i++) {
    sprintf(hex_output + i * 2, "%02x", hmac[i]);
  }
  hex_output[64] = '\0';

  return {true, std::string(hex_output)};
}

bool HttpManager::post(const String device_id, const String &signature, const String &url, const String &endpoint, const String &payload)
{
    const String fullUrl = url + endpoint; // Construct the full URL

    http.begin(fullUrl); // Initialize the HTTP connection

    http.addHeader("X-Device-ID", device_id);           // Set the device ID header
    http.addHeader("X-Signature", signature);           // Set the signature header
    http.addHeader("Content-Type", "application/json"); // Set the content type to JSON

    int responseCode = http.POST(payload); // Send the POST request with the payload
    if (responseCode >= 200 && responseCode < 300)
    {
        DEBUG_LOG_LN("HTTPS POST response code: " + String(responseCode));
        // If 204 No Content, do not attempt to read the response body which may not exist
        if (responseCode == 204) {
            DEBUG_LOG_LN("No content (204) — skipping response body.");
        } else {
            String responseBody = http.getString(); // Get the response body
            DEBUG_LOG_LN("Response body: " + responseBody);
        }

        http.end(); // End the HTTP connection
        return true;
    }
    else
    {
        DEBUG_LOG_LN("HTTPS POST response code: " + String(responseCode));
        String responseBody = http.getString(); // Get the response body
        DEBUG_LOG_LN("Response body: " + responseBody);

        http.end(); // End the HTTP connection
        return false;
    }
}