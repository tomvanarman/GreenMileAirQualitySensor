#include "SPS30.h"

bool SPS30::begin(TwoWire &wire, uint8_t maxRetries, bool doScan)
{
    if (_initialized)
        return true;

    _wire = &wire;

    if (doScan)
    {
        DEBUG_SECTION("I2C Scan");
        int deviceCount = 0;
        for (uint8_t address = 1; address < 127; address++)
        {
            _wire->beginTransmission(address);
            if (_wire->endTransmission() == 0)
            {
                DEBUG_KV("I2C device", String("0x") + String(address, HEX));
                deviceCount++;
            }
        }
        if (deviceCount == 0)
        {
            DEBUG_WARN("No I2C devices found");
        }
    }

    // Try initialize and start measurements
    for (uint8_t attempt = 0; attempt < maxRetries; ++attempt)
    {
        if (!initSPS30())
        {
            DEBUG_WARN(String("SPS30 init attempt ") + String(attempt + 1) + " failed");
            delay(200 * (attempt + 1));
            continue;
        }

        if (!startMeasurement())
        {
            DEBUG_WARN(String("SPS30 start attempt ") + String(attempt + 1) + " failed");
            delay(200 * (attempt + 1));
            continue;
        }

        DEBUG_INFO("SPS30 warming up (5s)...");
        delay(4500);

        _initialized = true;
        _measuring = true;
        DEBUG_OK("SPS30 initialized and measuring");
        return true;
    }

    DEBUG_FAIL("SPS30 initialization failed");
    return false;
}

bool SPS30::initSPS30()
{
    // Probe I2C first
    _wire->beginTransmission(SPS30_I2C_ADDRESS);
    if (_wire->endTransmission() != 0)
    {
        return false;
    }

    // Reset the sensor
    if (!sendCommand(SPS30_CMD_RESET))
    {
        return false;
    }
    delay(500);
    return true;
}

bool SPS30::startMeasurement()
{
    if (!_wire)
        return false;
    // Float format (0x0300)
    if (!sendCommandWithArg(SPS30_CMD_START_MEASUREMENT, 0x0300))
    {
        return false;
    }
    _measuring = true;
    return true;
}

bool SPS30::stopMeasurement()
{
    if (!_wire)
        return false;
    _measuring = false;
    return sendCommand(SPS30_CMD_STOP_MEASUREMENT);
}

bool SPS30::isDataReady()
{
    if (!_wire)
        return false;

    if (!sendCommand(SPS30_CMD_GET_DATA_READY))
    {
        return false;
    }

    uint8_t data[3];
    if (!readResponse(data, 3))
    {
        return false;
    }

    // Validate CRC
    if (calculateCRC(data[0], data[1]) != data[2])
    {
        return false;
    }

    uint16_t ready = (uint16_t(data[0]) << 8) | data[1];
    return ready == 1;
}

SPS30_measurement SPS30::readData(uint8_t maxRetries)
{
    SPS30_measurement out{};
    out.available = false;

    if (!_initialized || !_measuring)
    {
        DEBUG_WARN("SPS30 not initialized or not measuring");
        return out;
    }

    for (uint8_t attempt = 0; attempt < maxRetries; ++attempt)
    {
        if (!isDataReady())
        {
            delay(200);
            continue;
        }

        if (readMeasurement(out))
        {
            out.available = true;
            return out;
        }

        DEBUG_WARN(String("SPS30 read attempt ") + String(attempt + 1) + " failed");
        delay(200 * (attempt + 1));
    }

    DEBUG_FAIL("SPS30 read failed after retries");
    return out;
}

bool SPS30::readMeasurement(SPS30_measurement &out)
{
    if (!sendCommand(SPS30_CMD_READ_MEASUREMENT))
    {
        return false;
    }

    // 10 floats; each float is 2 words with CRC per word => 6 bytes per float -> 60 bytes
    uint8_t data[60];
    if (!readResponse(data, 60))
    {
        return false;
    }

    // Verify CRC and extract floats
    float values[10];
    for (int i = 0; i < 10; i++)
    {
        int offset = i * 6; // [b0 b1 crc] [b3 b4 crc]
        // CRC first word
        if (calculateCRC(data[offset], data[offset + 1]) != data[offset + 2])
        {
            return false;
        }
        // CRC second word
        if (calculateCRC(data[offset + 3], data[offset + 4]) != data[offset + 5])
        {
            return false;
        }
        uint8_t fb[4] = {data[offset], data[offset + 1], data[offset + 3], data[offset + 4]};
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

void SPS30::printMeasurement(const SPS30_measurement &m) const
{
    if (!m.available)
    {
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

bool SPS30::sendCommand(uint16_t command)
{
    _wire->beginTransmission(SPS30_I2C_ADDRESS);
    _wire->write(command >> 8);
    _wire->write(command & 0xFF);
    return _wire->endTransmission() == 0;
}

bool SPS30::sendCommandWithArg(uint16_t command, uint16_t arg)
{
    uint8_t crc = calculateCRC(arg >> 8, arg & 0xFF);

    _wire->beginTransmission(SPS30_I2C_ADDRESS);
    _wire->write(command >> 8);
    _wire->write(command & 0xFF);
    _wire->write(arg >> 8);
    _wire->write(arg & 0xFF);
    _wire->write(crc);
    return _wire->endTransmission() == 0;
}

bool SPS30::readResponse(uint8_t *data, uint16_t length)
{
    uint16_t bytesRead = _wire->requestFrom(SPS30_I2C_ADDRESS, static_cast<size_t>(length));
    if (bytesRead != length)
    {
        return false;
    }
    for (uint16_t i = 0; i < length; i++)
    {
        data[i] = _wire->read();
    }
    return true;
}

uint8_t SPS30::calculateCRC(uint8_t data1, uint8_t data2)
{
    uint8_t crc = 0xFF;
    uint8_t d[2] = {data1, data2};
    for (int i = 0; i < 2; i++)
    {
        crc ^= d[i];
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ 0x31;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

float SPS30::bytesToFloat(uint8_t *bytes)
{
    union
    {
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
