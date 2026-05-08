#pragma once // Only include this file once
#include <Arduino.h>
#include <Wire.h>
#include "DEBUG.h"

// SPS30 I2C Configuration
#define SPS30_I2C_ADDRESS 0x69
#define SPS30_MAX_SERIAL_LEN 32

// SPS30 Commands (16-bit)
#define SPS30_CMD_START_MEASUREMENT 0x0010
#define SPS30_CMD_STOP_MEASUREMENT  0x0104
#define SPS30_CMD_READ_MEASUREMENT  0x0300
#define SPS30_CMD_GET_DATA_READY    0x0202
#define SPS30_CMD_DEVICE_INFO       0xD003
#define SPS30_CMD_RESET             0xD304

// Single measurement payload
struct SPS30_measurement {
  float mc_1p0;                   // PM1.0 mass concentration [μg/m³]
  float mc_2p0;                   // PM2.5 mass concentration [μg/m³]
  float mc_4p0;                   // PM4.0 mass concentration [μg/m³]
  float mc_10p0;                  // PM10 mass concentration [μg/m³]
  float nc_0p5;                   // PM0.5 number concentration [#/cm³]
  float nc_1p0;                   // PM1.0 number concentration [#/cm³]
  float nc_2p5;                   // PM2.5 number concentration [#/cm³]
  float nc_4p0;                   // PM4.0 number concentration [#/cm³]
  float nc_10p0;                  // PM10 number concentration [#/cm³]
  float typical_particle_size;   // Typical particle size [μm]
  bool  available;               // Set true when a valid reading is present
};

class SPS30 {
public:
  SPS30() = default;

  // Initialize device on the given I2C bus, reset, start measurement and warm up.
  // doScan prints an I2C scan (optional).
  bool begin(TwoWire &wire, uint8_t maxRetries = 3, bool doScan = false);

  // Start/stop continuous measurement (float mode).
  bool startMeasurement();
  bool stopMeasurement();

  // Read a single sample if data ready. Returns struct with available=false if not ready or on error.
  SPS30_measurement readData(uint8_t maxRetries = 3);

  // Optional helpers
  bool isInitialized() const { return _initialized; }
  bool isDataReady();
  void printMeasurement(const SPS30_measurement &m) const;

  // Timestamp for the last successful read (millis)
  unsigned long last_update = 0;
  unsigned long last_measurement = 0;

private:
  // Low-level helpers
  bool initSPS30();
  bool readMeasurement(SPS30_measurement &out);
  bool sendCommand(uint16_t command);
  bool sendCommandWithArg(uint16_t command, uint16_t arg);
  bool readResponse(uint8_t *data, uint16_t length);
  static uint8_t calculateCRC(uint8_t data1, uint8_t data2);
  static float bytesToFloat(uint8_t *bytes);



private:
  TwoWire *_wire = nullptr;
  bool _initialized = false;
  bool _measuring = false;
};
