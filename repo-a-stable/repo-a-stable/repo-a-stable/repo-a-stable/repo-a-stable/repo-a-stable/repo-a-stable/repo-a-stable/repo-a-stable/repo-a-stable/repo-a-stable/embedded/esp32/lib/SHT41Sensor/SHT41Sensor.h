#ifndef LIB_SHT41SENSOR_SHT41Sensor_H_
#define LIB_SHT41SENSOR_SHT41Sensor_H_

#include <Adafruit_SHT4x.h>
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

  unsigned long last_measurement = 0;
  unsigned long last_update = 0;

private:
  Adafruit_SHT4x _sht4;
  bool _initialized = false;
};

#endif // LIB_SHT41SENSOR_SHT41Sensor_H_
