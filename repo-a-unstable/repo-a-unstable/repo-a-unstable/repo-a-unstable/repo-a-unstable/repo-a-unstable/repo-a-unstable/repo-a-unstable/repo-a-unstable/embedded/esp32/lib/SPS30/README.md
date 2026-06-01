# SPS30Sensor Library Documentation

## Overview
This library provides an interface for the Sensirion SPS30 particulate matter sensor using I2C on Arduino-compatible platforms. It supports initialization, measurement, and data interpretation for air quality monitoring.

---

## SPS30_measurement Struct
Holds a single measurement from the SPS30 sensor.

| Field                   | Type   | Description                                 |
|------------------------|--------|---------------------------------------------|
| mc_1p0                 | float  | PM1.0 mass concentration [μg/m³]            |
| mc_2p0                 | float  | PM2.5 mass concentration [μg/m³]            |
| mc_4p0                 | float  | PM4.0 mass concentration [μg/m³]            |
| mc_10p0                | float  | PM10 mass concentration [μg/m³]             |
| nc_0p5                 | float  | PM0.5 number concentration [#/cm³]          |
| nc_1p0                 | float  | PM1.0 number concentration [#/cm³]          |
| nc_2p5                 | float  | PM2.5 number concentration [#/cm³]          |
| nc_4p0                 | float  | PM4.0 number concentration [#/cm³]          |
| nc_10p0                | float  | PM10 number concentration [#/cm³]           |
| typical_particle_size  | float  | Typical particle size [μm]                  |
| available              | bool   | True if the measurement is valid            |

---

## SPS30 Class
### Methods
- **begin(TwoWire &wire, uint8_t maxRetries = 3, bool doScan = false)**
  - Initializes the sensor, performs optional I2C scan, resets and starts measurement.
- **startMeasurement() / stopMeasurement()**
  - Start or stop continuous measurement mode.
- **readData(uint8_t maxRetries = 3)**
  - Reads a measurement if data is ready. Returns a `SPS30_measurement` struct.
- **isInitialized()**
  - Returns true if the sensor is initialized.
- **isDataReady()**
  - Returns true if new data is available.
- **printMeasurement(const SPS30_measurement &m) const**
  - Prints a formatted summary of the measurement, including air quality assessment.

### Private Methods
- **initSPS30()**: Low-level initialization and reset.
- **readMeasurement(SPS30_measurement &out)**: Reads and parses raw sensor data.
- **sendCommand / sendCommandWithArg**: Sends commands to the sensor.
- **readResponse**: Reads I2C response.
- **calculateCRC**: Validates data integrity.
- **bytesToFloat**: Converts raw bytes to float.

---

## Example Usage
```cpp
#include <Wire.h>
#include "SPS30Sensor.h"

SPS30 sps30;

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9);
  sps30.begin(Wire, 3, true);
}

void loop() {
  SPS30_measurement m = sps30.readData();
  sps30.printMeasurement(m);
  delay(5000);
}
```

---

## Air Quality Assessment
The `printMeasurement` function includes a simple air quality assessment based on PM2.5:
- ≤12 μg/m³: Good
- ≤35 μg/m³: Moderate
- ≤55 μg/m³: Unhealthy for sensitive groups
- ≤150 μg/m³: Unhealthy
- >150 μg/m³: Very Unhealthy

(These are Air Quality Standards)

---

## SPS30 Command Codes
The SPS30 sensor uses specific 16-bit command codes for I2C communication. These are used to control the sensor and retrieve data:

| Command Name            | Code (Hex) | Description                                 |
|------------------------ |------------|---------------------------------------------|
| START_MEASUREMENT       | 0x0010     | Start continuous measurement (float mode)    |
| STOP_MEASUREMENT        | 0x0104     | Stop measurement                            |
| READ_MEASUREMENT        | 0x0300     | Read measured values                        |
| GET_DATA_READY          | 0x0202     | Check if new data is ready                   |
| DEVICE_INFO             | 0xD003     | Get device information (serial, etc.)        |
| RESET                   | 0xD304     | Soft reset the sensor                       |

These codes are defined in the [library](https://cdn.sparkfun.com/assets/2/d/2/a/6/Sensirion_SPS30_Particulate_Matter_Sensor_v0.9_D1__1_.pdf) and used internally for sensor control and data retrieval.

