#include <DEBUG.h>
#include <Handler.h>

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
        startErrorTask(rgb, SetupError::INVALID_CREDENTIALS);

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

bool Handler::setupSPS30(SPS30& sps30, TwoWire& wire, RGBLight& rgb) {
    while (!sps30.begin(wire, 1)) {
        DEBUG_WARN("SPS30 not found yet, retrying");
        wait(2000);
    }
    disableRGB(rgb);
    return true;
}

bool Handler::setupSHT41(SHT41Sensor& sht41, TwoWire& wire, RGBLight& rgb) {
    rgb.disable();

    if (!sht41.begin(wire, 3)) {
        DEBUG_WARN(
            "SHT41 not ready during setup, continuing with loop retries");
        return false;
    }
    disableRGB(rgb);
    return true;
}

void Handler::enterDeepSleep(LEDStrip& strip, SegmentDisplay& segmentDisplay,
                             bool useSIM) {
    deepSleepManager.enterDeepSleep(strip, segmentDisplay, useSIM);
}
