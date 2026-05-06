#include "SHT41Sensor.h"
#include "DEBUG.h"

SHT41Sensor::SHT41Sensor() {}

bool SHT41Sensor::begin(TwoWire &wire, uint8_t maxRetries) {
  if (_initialized) {
    return true;
  }

  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    // I2C address is 0x44 (fixed) for SHT41
    if (_sht4.begin(&wire)) {
      _sht4.setPrecision(SHT4X_HIGH_PRECISION);
      _initialized = true;
    }

    // Verify if sensor is working
    SHT41Data data = readData(2);
    if (data.available) {
      DEBUG_OK("SHT41 initialized successfully");
      delay(4600); // Wait for sensor to stabilize
      return true;
    }
    delay(100 * (attempt + 1));
    DEBUG_WARN(String("SHT41 init attempt ") + String(attempt + 1) + " failed");
  }

  DEBUG_FAIL("SHT41 initialization failed after all retries");
  return false;
}

/// @brief
/// @param readInterval in seconds
/// @return
SHT41Data SHT41Sensor::readData(uint8_t maxRetries) {
  SHT41Data data = {0, 0, false};

  if (!_initialized) {
    DEBUG_WARN("SHT41 not initialized");
    return data;
  }

  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    sensors_event_t humidity, temp;

    if (_sht4.getEvent(&humidity, &temp)) {
      data.temperature = temp.temperature;
      data.humidity = humidity.relative_humidity;
      data.available = true;
      return data;
    }

    delay(50 * (attempt + 1));
    DEBUG_WARN(String("SHT41 read attempt ") + String(attempt + 1) + " failed");
  }
  DEBUG_FAIL("Failed to read from sensor after retries");

  return data;
}
