#ifndef LIB_SHT41SENSOR_SHT41Sensor_H_
#define LIB_SHT41SENSOR_SHT41Sensor_H_

#include <Arduino.h>
#include <Wire.h>
// #include <TimeLib.h>

struct SHT41Data
{
  float temperature;
  float humidity;
  bool available;
  // uint8_t errorCode;
};

class SHT41Sensor
{
public:
  SHT41Sensor();

  bool begin(TwoWire &wire, uint8_t maxRetries = 3);
  SHT41Data readData(uint8_t maxRetries = 3);
  bool isInitialized() { return _initialized; }
  void resetState();

  unsigned long last_measurement = 0;
  unsigned long last_update = 0;

private:
  bool readRawMeasurement(uint16_t &rawTemperature, uint16_t &rawHumidity);
  static uint8_t calculateCRC(uint8_t firstByte, uint8_t secondByte);

  TwoWire *_wire = nullptr;
  bool _initialized = false;
};

#endif // LIB_SHT41SENSOR_SHT41Sensor_H_
