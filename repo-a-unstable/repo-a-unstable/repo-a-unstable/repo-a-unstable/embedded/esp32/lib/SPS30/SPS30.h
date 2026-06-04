#pragma once

#include <Arduino.h>

#include "DEBUG.h"

// SPS30 UART / SHDLC command codes
#define SPS30_UART_BAUD 115200
#define SPS30_CMD_START_MEASUREMENT 0x00
#define SPS30_CMD_STOP_MEASUREMENT 0x01
#define SPS30_CMD_DATA_READY 0x02
#define SPS30_CMD_READ_MEASUREMENT 0x03
#define SPS30_CMD_SLEEP 0x10
#define SPS30_CMD_WAKE_UP 0x11
#define SPS30_CMD_RESET 0xD3

// Single measurement result
struct SPS30_measurement {
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

class SPS30 {
public:
    SPS30() = default;
    bool begin(HardwareSerial& serial, int rxPin, int txPin,
               uint8_t maxRetries = 3);
    bool startMeasurement();
    bool stopMeasurement();
    bool sleep();
    bool wakeUp();
    SPS30_measurement readData(uint8_t maxRetries = 3);
    bool isInitialized() const {
        return _initialized;
    }
    uint32_t measurementAgeMs() const;
    void resetState();
    bool isDataReady();
    void printMeasurement(const SPS30_measurement& m) const;
    unsigned long last_update = 0;
    unsigned long last_measurement = 0;
    bool debug_raw_next_read = false;

private:
    bool initSPS30();
    bool readMeasurement(SPS30_measurement& out);
    bool sendCommand(uint8_t command);
    bool sendCommandWithData(uint8_t command, const uint8_t* data,
                             size_t dataLength);
    bool executeCommand(uint8_t command, const uint8_t* data, size_t dataLength,
                        uint8_t* response, size_t responseMax,
                        size_t* responseLength, uint32_t timeoutMs = 300,
                        bool logErrors = true);
    bool readFrame(uint8_t* frame, size_t frameMax, size_t* frameLength,
                   uint32_t timeoutMs);
    void writeFrameByte(uint8_t value);
    static uint8_t calculateChecksum(const uint8_t* data, size_t length);
    static float bytesToFloat(uint8_t* bytes);
    HardwareSerial* _serial = nullptr;
    uint32_t _measurementStartedAtMs = 0;
    bool _initialized = false;
    bool _measuring = false;
};
