#include <DEBUG.h>
#include <Handler.h>

namespace {

    void probeI2CDevice(TwoWire& wire, uint8_t address, const char* name) {
    wire.beginTransmission(address);
    uint8_t error = wire.endTransmission();

    String addressText = "0x";
    if (address < 16) {
        addressText += "0";
    }
    addressText += String(address, HEX);
    addressText.toUpperCase();

    if (error == 0) {
        DEBUG_OK(String(name) + " found at " + addressText);
        return;
    }

    DEBUG_WARN(String(name) + " missing at " + addressText + ", error " +
               String(error));
}
}

void Handler::setupSim7080(SIM7080& sim7080, RGBLight& rgb) {
    sim7080.initialize();
    sim7080.startModem();
    sim7080.setupNetwork();

    if (!sim7080.ensureConnected()) {
        DEBUG_WARN("Failed to connect to network");
        errorEncounteredRGB(rgb, SetupError::WIFI_INIT_FAILED);

        while (1)
            sim7080.ensureConnected();
    }
    disableRGB(rgb);
}

void Handler::setupRGB(RGBLight& rgb) {
    rgb.setup();
}

void Handler::rgbInitialization(RGBLight& rgb) {
    if (rgbTaskHandle == nullptr) {
        BaseType_t created =
            xTaskCreate(rgbTask, "RGBInitTask", 2048, &rgb, 1, &rgbTaskHandle);
        if (created != pdPASS) {
            DEBUG_WARN("RGB initialization task could not be started");
            return;
        }
        vTaskDelay(1);
    }
}

void Handler::disableRGB(RGBLight& rgb) {
    if (rgbTaskHandle != nullptr) {
        vTaskDelete(rgbTaskHandle);
        rgbTaskHandle = nullptr;
    }
    rgb.disable();
}

void Handler::rgbTask(void* pvParameters) {
    RGBLight* rgb = static_cast<RGBLight*>(pvParameters);
    if (rgb) {
        rgb->startInitialization();
    }
    vTaskDelete(nullptr);
}

void Handler::errorEncounteredRGB(RGBLight& rgb, SetupError error) {
    startErrorTask(rgb, error);
}

void Handler::errorTask(void* pvParameters) {
    ErrorTaskParams* params = static_cast<ErrorTaskParams*>(pvParameters);
    if (params && params->rgb) {
        params->rgb->errorEncountered(params->error);
    }
    delete params;
    vTaskDelete(nullptr);
}

void Handler::startErrorTask(RGBLight& rgb, SetupError error) {
    disableRGB(rgb);
    if (errorTaskHandle == nullptr) {
        ErrorTaskParams* params = new ErrorTaskParams{&rgb, error};
        xTaskCreate(errorTask, "ErrorTask", 2048, params, 1, &errorTaskHandle);
        DEBUG_INFO("Error display task started");
    }
}

void Handler::setupCredentialManager(CredentialManager& credential_manager,
                                     NetworkServer& server, RGBLight& rgb) {
    credential_manager.LoadCredentials();

    if (!credential_manager.ValidateDeviceCredentials()) {
        DEBUG_WARN(
            "Invalid or missing device credentials, starting AP for "
            "configuration...");
        server.StartAP();

        while (1) {
            server.HandleRequests();
            wait(50);
            yield();
        }
    }
    disableRGB(rgb);
}

void Handler::setupWifi(WiFiManager& network, NetworkServer& server,
                        RGBLight& rgb) {
    network.Connect();

    if (!network.isConnected()) {
        DEBUG_WARN("Wrong credentials, starting AP for configuration...");
        server.StartAP();
        startErrorTask(rgb, SetupError::INVALID_CREDENTIALS);

        while (true) {
            server.HandleRequests();
            wait(50);
            yield();
        }
    }
    disableRGB(rgb);
}

bool Handler::setupSPS30(SPS30& sps30, HardwareSerial& serial, int rxPin,
                         int txPin, RGBLight& rgb) {
    if (!sps30.begin(serial, rxPin, txPin, 1)) {
        DEBUG_WARN(
            "SPS30 not ready during setup, continuing with loop retries");
        rgb.errorEncountered(SetupError::SENSOR_INIT_FAILED);
        return false;
    }
    disableRGB(rgb);
    return true;
}

bool Handler::setupSHT41(SHT41Sensor& sht41, TwoWire& wire, RGBLight& rgb) {
    rgb.disable();

    if (!sht41.begin(wire, 3)) {
        DEBUG_WARN(
            "SHT41 not ready during setup, continuing with loop retries");
        rgb.errorEncountered(SetupError::SENSOR_INIT_FAILED);
        return false;
    }
    disableRGB(rgb);
    return true;
}

void Handler::setupSensorI2CBus(TwoWire& wire, int sdaPin, int sclPin,
                                uint32_t clockHz, uint32_t timeoutMs) {
    DEBUG_TRACE_SECTION("Sensor I2C Setup");
    DEBUG_TRACE_KV("Sensor SDA pin", sdaPin);
    DEBUG_TRACE_KV("Sensor SCL pin", sclPin);

    DEBUG_TRACE_SECTION("Sensor I2C Bus Recovery");
    wire.end();

    pinMode(sdaPin, INPUT_PULLUP);
    pinMode(sclPin, INPUT_PULLUP);
    wait(10);

    bool sdaHigh = digitalRead(sdaPin) == HIGH;
    bool sclHigh = digitalRead(sclPin) == HIGH;
    DEBUG_TRACE_KV("SDA idle", sdaHigh ? "HIGH" : "LOW");
    DEBUG_TRACE_KV("SCL idle", sclHigh ? "HIGH" : "LOW");

    if (!sdaHigh) {
        DEBUG_WARN("SDA is low, pulsing SCL to release the I2C bus");
        pinMode(sclPin, OUTPUT_OPEN_DRAIN);

        for (uint8_t pulse = 0; pulse < 9; pulse++) {
            digitalWrite(sclPin, LOW);
            delayMicroseconds(5);
            digitalWrite(sclPin, HIGH);
            delayMicroseconds(5);
        }
    }

    pinMode(sdaPin, INPUT_PULLUP);
    pinMode(sclPin, INPUT_PULLUP);

    wire.begin(sdaPin, sclPin);
    wire.setClock(clockHz);
    wire.setTimeOut(timeoutMs);

    DEBUG_TRACE_SECTION("I2C Known Device Probe");
    probeI2CDevice(wire, 0x44, "SHT41");
}

void Handler::enterDeepSleep(SPS30& sps30, TwoWire& sensorWire,
                             SIM7080& sim7080, int sensorSdaPin,
                             int sensorSclPin, bool useSIM) {
    deepSleepManager.enterDeepSleep(sps30, sensorWire, sim7080, sensorSdaPin,
                                    sensorSclPin, useSIM);
}
