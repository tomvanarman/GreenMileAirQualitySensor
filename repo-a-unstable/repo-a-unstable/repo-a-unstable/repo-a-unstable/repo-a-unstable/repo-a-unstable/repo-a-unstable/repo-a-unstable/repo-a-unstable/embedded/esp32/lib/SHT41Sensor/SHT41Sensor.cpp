#include "SHT41Sensor.h"

#include "DEBUG.h"
#include "HelpMethod.h"

namespace {
constexpr uint8_t kSht41Address = 0x44;
constexpr uint8_t kMeasureHighPrecision = 0xFD;
constexpr uint32_t kMeasurementDelayMs = 15;
}

SHT41Sensor::SHT41Sensor() {}

void SHT41Sensor::resetState() {
  _initialized = false;
}

bool SHT41Sensor::begin(TwoWire &wire, uint8_t maxRetries) {
  _wire = &wire;

  if (_initialized) {
    return true;
  }

  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    _wire->beginTransmission(kSht41Address);
    if (_wire->endTransmission() != 0) {
      DEBUG_WARN(String("SHT41 init attempt ") + String(attempt + 1) +
                 " failed");
      wait(100 * (attempt + 1));
      continue;
    }

    uint16_t rawTemperature = 0;
    uint16_t rawHumidity = 0;
    if (readRawMeasurement(rawTemperature, rawHumidity)) {
      _initialized = true;
      DEBUG_OK("SHT41 initialized successfully");
      return true;
    }

    DEBUG_WARN(String("SHT41 init attempt ") + String(attempt + 1) +
               " failed");
    wait(100 * (attempt + 1));
  }

  DEBUG_WARN("SHT41 initialization attempt batch failed");
  return false;
}

SHT41Data SHT41Sensor::readData(uint8_t maxRetries) {
  SHT41Data data = {0, 0, false};

  if (!_initialized || !_wire) {
    DEBUG_WARN("SHT41 not initialized");
    return data;
  }

  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    uint16_t rawTemperature = 0;
    uint16_t rawHumidity = 0;

    if (readRawMeasurement(rawTemperature, rawHumidity)) {
      data.temperature = -45.0f + 175.0f * rawTemperature / 65535.0f;
      data.humidity = -6.0f + 125.0f * rawHumidity / 65535.0f;
      data.humidity = constrain(data.humidity, 0.0f, 100.0f);
      data.available = true;
      return data;
    }

    DEBUG_WARN(String("SHT41 read attempt ") + String(attempt + 1) +
               " failed");
    wait(50 * (attempt + 1));
  }

  DEBUG_WARN("SHT41 read failed after retries");
  return data;
}

bool SHT41Sensor::readRawMeasurement(uint16_t &rawTemperature,
                                     uint16_t &rawHumidity) {
  if (!_wire) {
    return false;
  }

  _wire->beginTransmission(kSht41Address);
  _wire->write(kMeasureHighPrecision);
  if (_wire->endTransmission() != 0) {
    return false;
  }

  wait(kMeasurementDelayMs);

  uint8_t bytesRead = _wire->requestFrom(kSht41Address, static_cast<uint8_t>(6));
  if (bytesRead != 6) {
    while (_wire->available()) {
      _wire->read();
    }
    return false;
  }

  uint8_t buffer[6];
  for (uint8_t i = 0; i < sizeof(buffer); i++) {
    buffer[i] = _wire->read();
  }

  if (calculateCRC(buffer[0], buffer[1]) != buffer[2] ||
      calculateCRC(buffer[3], buffer[4]) != buffer[5]) {
    return false;
  }

  rawTemperature = (uint16_t(buffer[0]) << 8) | buffer[1];
  rawHumidity = (uint16_t(buffer[3]) << 8) | buffer[4];
  return true;
}

uint8_t SHT41Sensor::calculateCRC(uint8_t firstByte, uint8_t secondByte) {
  uint8_t crc = 0xFF;
  uint8_t data[2] = {firstByte, secondByte};

  for (uint8_t i = 0; i < sizeof(data); i++) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ 0x31;
      } else {
        crc <<= 1;
      }
    }
  }

  return crc;
}
