#include "SensorReadoutManager.h"

#include <math.h>

#include "DEBUG.h"
#include "JsonBuilder.h"

namespace {
constexpr uint32_t kDataMeasurementInterval = 0;
constexpr uint32_t kSps30WarmupMs = 10000;
constexpr uint8_t kRequiredStableSps30Readings = 1;
}  

SensorReadoutManager::SensorReadoutManager(SPS30& sps30, SHT41Sensor& sht41,
                                           PayloadPublisher& publisher,
                                           TimeService& timeService,
                                           SIM7080& sim7080, bool& useSIM,
                                           HardwareSerial& sps30Serial,
                                           TwoWire& sensorWire, int sps30RxPin,
                                           int sps30TxPin)
    : sps30_(sps30),
      sht41_(sht41),
      publisher_(publisher),
      timeService_(timeService),
      sim7080_(sim7080),
      useSIM_(useSIM),
      sps30Serial_(sps30Serial),
      sensorWire_(sensorWire),
      sps30RxPin_(sps30RxPin),
      sps30TxPin_(sps30TxPin) {}

void SensorReadoutManager::handle(String deviceId) {
    if (!sps30SentThisWake_) {
        sps30SentThisWake_ = handleSPS30(deviceId);
    }

    if (!sht41SentThisWake_) {
        sht41SentThisWake_ = handleSHT41(deviceId);
    }
}

bool SensorReadoutManager::allSent() const {
    return sps30SentThisWake_ && sht41SentThisWake_;
}

bool SensorReadoutManager::handleSPS30(String deviceId) {
    uint32_t now = millis();
    bool canMeasure = now - sps30_.last_measurement >= kDataMeasurementInterval;

    if (!canMeasure)
        return false;

    if (!sps30_.isInitialized()) {
        DEBUG_WARN("SPS30 not initialized, retrying setup");
        sps30_.begin(sps30Serial_, sps30RxPin_, sps30TxPin_, 1);
        return false;
    }

    if (sps30_.measurementAgeMs() < kSps30WarmupMs) {
        DEBUG_TRACE_KV("SPS30 warmup ms remaining",
                       kSps30WarmupMs - sps30_.measurementAgeMs());
        return false;
    }

    if (!sps30WarmupLogged_) {
        DEBUG_OK("SPS30 warmup completed");
        sps30WarmupLogged_ = true;
    }

    SPS30_measurement spsData = sps30_.readData();

    if (!spsData.available) {
        DEBUG_WARN("SPS30 read not ready");
        return false;
    }

    if (!isValidSPS30Measurement(spsData)) {
        DEBUG_WARN("Rejected invalid SPS30 measurement");
        DEBUG_KV("SPS30 rejected PM2.5", spsData.mc_2p0);
        DEBUG_KV("SPS30 rejected PM10", spsData.mc_10p0);
        DEBUG_KV("SPS30 rejected size", spsData.typical_particle_size);
        sps30_.debug_raw_next_read = true;
        stableSps30Readings_ = 0;
        return false;
    }

    lastGoodSps30Data_ = spsData;
    stableSps30Readings_++;

    if (stableSps30Readings_ < kRequiredStableSps30Readings) {
        DEBUG_WARN("Waiting for stable SPS30 readings");
        sps30_.last_measurement = now;
        return false;
    }

    sps30_.last_measurement = now;
    sps30_.last_update = now;

    String data = JsonBuilder::BuildJson([&](JsonDocument& doc) {
        doc["Device_id"] = deviceId;
        doc["mc_1p0"] = spsData.mc_1p0;
        doc["mc_2p0"] = spsData.mc_2p0;
        doc["mc_4p0"] = spsData.mc_4p0;
        doc["mc_10p0"] = spsData.mc_10p0;
        doc["nc_0p5"] = spsData.nc_0p5;
        doc["nc_1p0"] = spsData.nc_1p0;
        doc["nc_2p5"] = spsData.nc_2p5;
        doc["nc_4p0"] = spsData.nc_4p0;
        doc["nc_10p0"] = spsData.nc_10p0;
        doc["typical_particle_size"] = spsData.typical_particle_size;
        doc["time_unix"] = currentPayloadUnixTime();
    });
    DEBUG_KV("SPS30 payload", data);

    bool sent = publisher_.sendPayload(PayloadPublisher::sps30Path(), data);

    if (sent) {
        DEBUG_OK("SPS30 data sent");
        return true;
    }

    DEBUG_WARN("Failed to send SPS30 data");
    return false;
}

bool SensorReadoutManager::handleSHT41(String deviceId) {
    uint32_t now = millis();
    bool canMeasure = now - sht41_.last_measurement >= kDataMeasurementInterval;

    if (!canMeasure)
        return false;

    if (!sht41_.isInitialized()) {
        DEBUG_WARN("SHT41 not initialized, retrying setup");
        sht41_.begin(sensorWire_, 1);
        return false;
    }

    SHT41Data shtData = sht41_.readData(3);

    if (!shtData.available) {
        DEBUG_WARN("Failed to read from SHT41 sensor");
        if (lastGoodSht41Data_.available) {
            DEBUG_WARN("Keeping last good SHT41 reading");
            sht41_.last_measurement = now;
        }
        return false;
    }

    if (!isValidSHT41Measurement(shtData)) {
        DEBUG_WARN("Rejected invalid SHT41 measurement");
        return false;
    }

    lastGoodSht41Data_ = shtData;
    sht41_.last_measurement = now;
    sht41_.last_update = now;

    String data = JsonBuilder::BuildJson([&](JsonDocument& doc) {
        doc["Device_id"] = deviceId;
        doc["temperature"] = shtData.temperature;
        doc["humidity"] = shtData.humidity;
        doc["time_unix"] = currentPayloadUnixTime();
    });
    DEBUG_KV("SHT41 payload", data);

    bool sent = publisher_.sendPayload(PayloadPublisher::sht41Path(), data);

    if (sent) {
        DEBUG_OK("SHT41 data sent");
        return true;
    }

    DEBUG_WARN("Failed to send SHT41 data");
    return false;
}

bool SensorReadoutManager::isValidSPS30Measurement(
    const SPS30_measurement& data) const {
    constexpr float kMaxMassConcentration = 1000.0f;
    constexpr float kMaxNumberConcentration = 100000.0f;
    constexpr float kMinParticleSize = 0.1f;
    constexpr float kMaxParticleSize = 10.0f;

    if (!isfinite(data.mc_1p0) || data.mc_1p0 < 0.0f ||
        !isfinite(data.mc_2p0) || data.mc_2p0 < 0.0f ||
        !isfinite(data.mc_4p0) || data.mc_4p0 < 0.0f ||
        !isfinite(data.mc_10p0) || data.mc_10p0 < 0.0f ||
        !isfinite(data.nc_0p5) || data.nc_0p5 < 0.0f ||
        !isfinite(data.nc_1p0) || data.nc_1p0 < 0.0f ||
        !isfinite(data.nc_2p5) || data.nc_2p5 < 0.0f ||
        !isfinite(data.nc_4p0) || data.nc_4p0 < 0.0f ||
        !isfinite(data.nc_10p0) || data.nc_10p0 < 0.0f ||
        !isfinite(data.typical_particle_size)) {
        return false;
    }

    if (data.mc_1p0 > kMaxMassConcentration ||
        data.mc_2p0 > kMaxMassConcentration ||
        data.mc_4p0 > kMaxMassConcentration ||
        data.mc_10p0 > kMaxMassConcentration ||
        data.nc_0p5 > kMaxNumberConcentration ||
        data.nc_1p0 > kMaxNumberConcentration ||
        data.nc_2p5 > kMaxNumberConcentration ||
        data.nc_4p0 > kMaxNumberConcentration ||
        data.nc_10p0 > kMaxNumberConcentration) {
        return false;
    }

    return data.typical_particle_size >= kMinParticleSize &&
           data.typical_particle_size <= kMaxParticleSize;
}

bool SensorReadoutManager::isValidSHT41Measurement(
    const SHT41Data& data) const {
    return isfinite(data.temperature) && isfinite(data.humidity) &&
           data.temperature >= -40.0f && data.temperature <= 125.0f &&
           data.humidity >= 0.0f && data.humidity <= 100.0f;
}

uint64_t SensorReadoutManager::currentPayloadUnixTime() const {
    if (useSIM_)
        return sim7080_.getCurrentTimestampMs() / 1000;
    return timeService_.currentTimestampMs() / 1000;
}
