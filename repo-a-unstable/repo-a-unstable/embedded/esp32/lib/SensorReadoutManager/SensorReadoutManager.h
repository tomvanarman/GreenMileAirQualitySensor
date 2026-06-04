#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "../PayloadPublisher/PayloadPublisher.h"
#include "../SHT41Sensor/SHT41Sensor.h"
#include "../SIM7080/SIM7080.h"
#include "../SPS30/SPS30.h"
#include "../TimeService/TimeService.h"

// clang-format off
class SensorReadoutManager {
 public:
    SensorReadoutManager(
        SPS30& sps30,                 // NOLINT(runtime/references)
        SHT41Sensor& sht41,           // NOLINT(runtime/references)
        PayloadPublisher& publisher,  // NOLINT(runtime/references)
        TimeService& timeService,     // NOLINT(runtime/references)
        SIM7080& sim7080,             // NOLINT(runtime/references)
        bool& useSIM,                 // NOLINT(runtime/references)
        HardwareSerial& sps30Serial,  // NOLINT(runtime/references)
        TwoWire& sensorWire,          // NOLINT(runtime/references)
        int sps30RxPin, int sps30TxPin);

    void handle(String deviceId);
    bool allSent() const;

 private:

    bool handleSPS30(String deviceId);
    bool handleSHT41(String deviceId);
    bool isValidSPS30Measurement(const SPS30_measurement& data) const;
    bool isValidSHT41Measurement(const SHT41Data& data) const;
    uint64_t currentPayloadUnixTime() const;

    SPS30& sps30_;
    SHT41Sensor& sht41_;
    PayloadPublisher& publisher_;
    TimeService& timeService_;
    SIM7080& sim7080_;
    bool& useSIM_;
    HardwareSerial& sps30Serial_;
    TwoWire& sensorWire_;
    int sps30RxPin_;
    int sps30TxPin_;
    SHT41Data lastGoodSht41Data_ = {0, 0, false};
    SPS30_measurement lastGoodSps30Data_ = {};
    uint8_t stableSps30Readings_ = 0;
  bool sps30SentThisWake_ = false;
  bool sht41SentThisWake_ = false;
  bool sps30WarmupLogged_ = false;
};
// clang-format on
