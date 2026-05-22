#pragma once // Only include this file once
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "DEBUG.h"

// SPS30 I2C Configuration
// SPS30 I2C address and config
#define SPS30_I2C_ADDRESS 0x69
#define SPS30_MAX_SERIAL_LEN 32

// SPS30 command codes
#define SPS30_CMD_START_MEASUREMENT 0x0010
#define SPS30_CMD_STOP_MEASUREMENT 0x0104
#define SPS30_CMD_READ_MEASUREMENT 0x0300
#define SPS30_CMD_GET_DATA_READY 0x0202
#define SPS30_CMD_SLEEP 0x1001
#define SPS30_CMD_WAKE_UP 0x1103
#define SPS30_CMD_DEVICE_INFO 0xD003
#define SPS30_CMD_RESET 0xD304

// Single measurement result
struct SPS30_measurement
{
  float mc_1p0;
  float mc_2p0;
  float mc_4p0;
  float mc_10p0;
  float nc_0p5;
  float nc_1p0;
  float nc_2p5;
  float nc_4p0;
  float nc_10p0;
  float typical_particle_size;
  bool available; 
};

class SPS30
{
public:
  SPS30() = default;
  bool begin(TwoWire &wire, uint8_t maxRetries = 3, bool doScan = false);
  bool startMeasurement();
  bool stopMeasurement();
  bool sleep();
  bool wakeUp(TwoWire &wire);
  SPS30_measurement readData(uint8_t maxRetries = 3);
  bool isInitialized() const { return _initialized; }
  void resetState();
  bool isDataReady();
  void printMeasurement(const SPS30_measurement &m) const;
  unsigned long last_update = 0;
  unsigned long last_measurement = 0;
  bool debug_raw_next_read = false;

private:
  bool initSPS30();
  bool readMeasurement(SPS30_measurement &out);
  bool sendCommand(uint16_t command);
  bool sendCommandWithArg(uint16_t command, uint16_t arg);
  bool readResponse(uint8_t *data, uint16_t length);
  static uint8_t calculateCRC(uint8_t data1, uint8_t data2);
  static float bytesToFloat(uint8_t *bytes);
  TwoWire *_wire = nullptr;
  bool _initialized = false;
  bool _measuring = false;
};
