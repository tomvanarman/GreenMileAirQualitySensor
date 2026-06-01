#include "SPS30.h"

#include "HelpMethod.h"

namespace {
constexpr uint8_t kFrameDelimiter = 0x7E;
constexpr uint8_t kFrameEscape = 0x7D;
constexpr uint8_t kFrameXor = 0x20;
constexpr uint8_t kDeviceAddress = 0x00;
constexpr uint8_t kStatusOk = 0x00;
constexpr uint8_t kStartMeasurementData[] = {0x01, 0x03};
}

bool SPS30::begin(HardwareSerial& serial, int rxPin, int txPin,
                  uint8_t maxRetries) {
    if (_initialized)
        return true;

    _serial = &serial;
    _serial->begin(SPS30_UART_BAUD, SERIAL_8N1, rxPin, txPin);
    _serial->setTimeout(1000);

    wakeUp();

    // Try initialize and start measurements
    for (uint8_t attempt = 0; attempt < maxRetries; ++attempt) {
        if (!initSPS30()) {
            DEBUG_WARN(String("SPS30 init attempt ") + String(attempt + 1) +
                       " failed");
            yield();
            continue;
        }

        if (!startMeasurement()) {
            DEBUG_WARN(String("SPS30 start attempt ") + String(attempt + 1) +
                       " failed");
            yield();
            continue;
        }

        _initialized = true;
        _measuring = true;
        DEBUG_OK("SPS30 initialized and measuring");
        return true;
    }

    DEBUG_WARN("SPS30 initialization attempt batch failed");
    return false;
}

bool SPS30::initSPS30() {
    if (!sendCommand(SPS30_CMD_RESET)) {
        return false;
    }
    wait(50);
    return true;
}

bool SPS30::startMeasurement() {
    if (!_serial)
        return false;

    if (!sendCommandWithData(SPS30_CMD_START_MEASUREMENT, kStartMeasurementData,
                             sizeof(kStartMeasurementData))) {
        return false;
    }
    _measuring = true;
    _measurementStartedAtMs = millis();
    DEBUG_OK("SPS30 measurement mode started");
    return true;
}

bool SPS30::stopMeasurement() {
    if (!_serial)
        return false;
    _measuring = false;
    _measurementStartedAtMs = 0;
    return sendCommand(SPS30_CMD_STOP_MEASUREMENT);
}

bool SPS30::sleep() {
    if (!_serial)
        return false;

    if (_measuring) {
        stopMeasurement();
        wait(20);
    }

    bool slept = sendCommand(SPS30_CMD_SLEEP);
    if (slept) {
        _initialized = false;
        _measuring = false;
        _measurementStartedAtMs = 0;
    }
    return slept;
}

bool SPS30::wakeUp() {
    if (!_serial)
        return false;

    _serial->write(0xFF);
    _serial->flush();
    wait(10);

    bool woke = executeCommand(SPS30_CMD_WAKE_UP, nullptr, 0, nullptr, 0,
                               nullptr, 300, false);
    wait(10);
    return woke;
}

void SPS30::resetState() {
    _initialized = false;
    _measuring = false;
    _measurementStartedAtMs = 0;
}

uint32_t SPS30::measurementAgeMs() const {
    if (!_measuring || _measurementStartedAtMs == 0)
        return 0;
    return millis() - _measurementStartedAtMs;
}

bool SPS30::isDataReady() {
    if (!_initialized || !_measuring)
        return false;

    uint8_t data[4];
    size_t dataLength = 0;
    if (!executeCommand(SPS30_CMD_DATA_READY, nullptr, 0, data, sizeof(data),
                        &dataLength, 1000)) {
        return false;
    }

    if (dataLength == 0)
        return false;

    for (size_t i = 0; i < dataLength; i++) {
        if (data[i] != 0)
            return true;
    }
    return false;
}

SPS30_measurement SPS30::readData(uint8_t maxRetries) {
    SPS30_measurement out{};
    out.available = false;

    if (!_initialized || !_measuring) {
        DEBUG_WARN("SPS30 not initialized or not measuring");
        return out;
    }

    for (uint8_t attempt = 0; attempt < maxRetries; ++attempt) {
        if (readMeasurement(out)) {
            out.available = true;
            return out;
        }

        DEBUG_TRACE(String("SPS30 read attempt ") + String(attempt + 1) +
                    " failed");
        yield();
    }

    DEBUG_TRACE("SPS30 read failed after retries");
    return out;
}

bool SPS30::readMeasurement(SPS30_measurement& out) {
    uint8_t data[40];
    size_t dataLength = 0;
    if (!executeCommand(SPS30_CMD_READ_MEASUREMENT, nullptr, 0, data,
                        sizeof(data), &dataLength, 1000)) {
        return false;
    }

    if (dataLength == 0) {
        return false;
    }

    if (dataLength < 40) {
        DEBUG_WARN("SPS30 UART measurement response too short");
        return false;
    }

    if (debug_raw_next_read) {
        String raw = "";
        for (int i = 0; i < 18; i++) {
            if (data[i] < 16) {
                raw += "0";
            }
            raw += String(data[i], HEX);
            if (i < 17) {
                raw += " ";
            }
        }
        raw.toUpperCase();
        DEBUG_KV("SPS30 raw first 18 bytes", raw);
        debug_raw_next_read = false;
    }

    float values[10];
    for (int i = 0; i < 10; i++) {
        int offset = i * 4;
        uint8_t fb[4] = {data[offset], data[offset + 1], data[offset + 2],
                         data[offset + 3]};
        values[i] = bytesToFloat(fb);
    }

    out.mc_1p0 = values[0];
    out.mc_2p0 = values[1];
    out.mc_4p0 = values[2];
    out.mc_10p0 = values[3];
    out.nc_0p5 = values[4];
    out.nc_1p0 = values[5];
    out.nc_2p5 = values[6];
    out.nc_4p0 = values[7];
    out.nc_10p0 = values[8];
    out.typical_particle_size = values[9];
    return true;
}

void SPS30::printMeasurement(const SPS30_measurement& m) const {
    if (!m.available) {
        DEBUG_WARN("SPS30: no valid measurement to print");
        return;
    }

    DEBUG_SECTION("SPS30 Measurement");
    DEBUG_KV("PM1.0 Mass Concentration", String(m.mc_1p0) + " μg/m³");
    DEBUG_KV("PM2.5 Mass Concentration", String(m.mc_2p0) + " μg/m³");
    DEBUG_KV("PM4.0 Mass Concentration", String(m.mc_4p0) + " μg/m³");
    DEBUG_KV("PM10 Mass Concentration", String(m.mc_10p0) + " μg/m³");
    DEBUG_KV("PM0.5 Number Concentration", String(m.nc_0p5) + " #/cm³");
    DEBUG_KV("PM1.0 Number Concentration", String(m.nc_1p0) + " #/cm³");
    DEBUG_KV("PM2.5 Number Concentration", String(m.nc_2p5) + " #/cm³");
    DEBUG_KV("PM4.0 Number Concentration", String(m.nc_4p0) + " #/cm³");
    DEBUG_KV("PM10 Number Concentration", String(m.nc_10p0) + " #/cm³");
    DEBUG_KV("Typical Particle Size", String(m.typical_particle_size) + " μm");
}

bool SPS30::sendCommand(uint8_t command) {
    return sendCommandWithData(command, nullptr, 0);
}

bool SPS30::sendCommandWithData(uint8_t command, const uint8_t* data,
                                size_t dataLength) {
    return executeCommand(command, data, dataLength, nullptr, 0, nullptr);
}

bool SPS30::executeCommand(uint8_t command, const uint8_t* data,
                           size_t dataLength, uint8_t* response,
                           size_t responseMax, size_t* responseLength,
                           uint32_t timeoutMs, bool logErrors) {
    if (!_serial || dataLength > 255) {
        return false;
    }

    while (_serial->available()) {
        _serial->read();
    }

    uint8_t checksumData[3 + 255];
    checksumData[0] = kDeviceAddress;
    checksumData[1] = command;
    checksumData[2] = static_cast<uint8_t>(dataLength);
    for (size_t i = 0; i < dataLength; i++) {
        checksumData[3 + i] = data[i];
    }

    _serial->write(kFrameDelimiter);
    writeFrameByte(kDeviceAddress);
    writeFrameByte(command);
    writeFrameByte(static_cast<uint8_t>(dataLength));
    for (size_t i = 0; i < dataLength; i++) {
        writeFrameByte(data[i]);
    }
    writeFrameByte(calculateChecksum(checksumData, dataLength + 3));
    _serial->write(kFrameDelimiter);
    _serial->flush();

    uint8_t frame[64];
    size_t frameLength = 0;
    if (!readFrame(frame, sizeof(frame), &frameLength, timeoutMs)) {
        if (logErrors)
            DEBUG_WARN("SPS30 UART response timeout");
        return false;
    }

    if (frameLength < 5) {
        if (logErrors)
            DEBUG_WARN("SPS30 UART response too short");
        return false;
    }

    uint8_t checksum = calculateChecksum(frame, frameLength - 1);
    if (checksum != frame[frameLength - 1]) {
        if (logErrors)
            DEBUG_WARN("SPS30 UART checksum mismatch");
        return false;
    }

    uint8_t address = frame[0];
    uint8_t responseCommand = frame[1];
    uint8_t state = frame[2];
    uint8_t length = frame[3];

    if (address != kDeviceAddress || responseCommand != command) {
        if (logErrors)
            DEBUG_WARN("SPS30 UART response header mismatch");
        return false;
    }

    if (state != kStatusOk) {
        if (logErrors)
            DEBUG_WARN("SPS30 UART command failed with state " + String(state));
        return false;
    }

    if (frameLength != static_cast<size_t>(length) + 5) {
        if (logErrors)
            DEBUG_WARN("SPS30 UART response length mismatch");
        return false;
    }

    if (responseLength != nullptr) {
        *responseLength = length;
    }

    if (response != nullptr && responseMax > 0) {
        size_t copyLength = length < responseMax ? length : responseMax;
        memcpy(response, frame + 4, copyLength);
    }

    return true;
}

bool SPS30::readFrame(uint8_t* frame, size_t frameMax, size_t* frameLength,
                      uint32_t timeoutMs) {
    bool inFrame = false;
    bool escaped = false;
    size_t length = 0;
    uint32_t start = millis();

    while (millis() - start < timeoutMs) {
        if (!_serial->available()) {
            wait(1);
            continue;
        }

        uint8_t value = static_cast<uint8_t>(_serial->read());
        if (value == kFrameDelimiter) {
            if (inFrame && length > 0) {
                *frameLength = length;
                return true;
            }

            inFrame = true;
            escaped = false;
            length = 0;
            continue;
        }

        if (!inFrame) {
            continue;
        }

        if (escaped) {
            value ^= kFrameXor;
            escaped = false;
        } else if (value == kFrameEscape) {
            escaped = true;
            continue;
        }

        if (length >= frameMax) {
            DEBUG_WARN("SPS30 UART frame too large");
            return false;
        }

        frame[length++] = value;
    }

    return false;
}

void SPS30::writeFrameByte(uint8_t value) {
    if (value == kFrameDelimiter || value == kFrameEscape) {
        _serial->write(kFrameEscape);
        _serial->write(value ^ kFrameXor);
        return;
    }

    _serial->write(value);
}

uint8_t SPS30::calculateChecksum(const uint8_t* data, size_t length) {
    uint8_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += data[i];
    }
    return ~sum;
}

float SPS30::bytesToFloat(uint8_t* bytes) {
    union {
        float f;
        uint8_t b[4];
    } u;
    // Big-endian from SPS30 -> host float
    u.b[3] = bytes[0];
    u.b[2] = bytes[1];
    u.b[1] = bytes[2];
    u.b[0] = bytes[3];
    return u.f;
}
