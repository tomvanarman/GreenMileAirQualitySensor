#include <DEBUG.h>
#include <Handler.h>

void Handler::setupSim7080(SIM7080 &sim7080, RGBLight &rgb) {
  sim7080.initialize();
  sim7080.startModem();
  sim7080.setupNetwork();

  if (!sim7080.ensureConnected()) {
    DEBUG_WARN("Failed to connect to network");
    disableRGB(rgb);
    errorEncounteredRGB(rgb, SetupError::WIFI_INIT_FAILED);

    while (1)
      sim7080.ensureConnected();
  }
  disableRGB(rgb);
}

void Handler::setupRGB(RGBLight &rgb) {
  rgb.setup();
}


void Handler::rgbInitialization(RGBLight &rgb) {
  if (rgbTaskHandle == nullptr) {
    // Übergibt einen Zeiger auf das RGBLight-Objekt an den Task
    xTaskCreate(rgbTask, "RGBInitTask", 2048, &rgb, 1, &rgbTaskHandle);
    DEBUG_INFO("RGB initialization task started");
  }
}

void Handler::disableRGB(RGBLight &rgb) {
  if (rgbTaskHandle != nullptr) {
    vTaskDelete(rgbTaskHandle);
    rgbTaskHandle = nullptr;
  }
  rgb.disable();
}

void Handler::rgbTask(void *pvParameters) {
  RGBLight *rgb = static_cast<RGBLight*>(pvParameters);
  if (rgb) {
    rgb->startInitialization();
  }
  vTaskDelete(nullptr); // Task beendet sich selbst
}


void Handler::errorEncounteredRGB(RGBLight &rgb, SetupError error) {
  disableRGB(rgb);
  rgb.errorEncountered(error);
}

void Handler::setupCredentialManager(CredentialManager &credential_manager,
                                     NetworkServer &server, RGBLight &rgb) {
  credential_manager.LoadCredentials();

  if (!credential_manager.ValidateCredentials()) {
    DEBUG_WARN(
        "Invalid or missing credentials, starting AP for configuration...");
    server.StartAP();

      disableRGB(rgb);
      errorEncounteredRGB(rgb, SetupError::INVALID_CREDENTIALS);

    while (true) {
      server.HandleRequests();
      wait(10);
    }
  }
  disableRGB(rgb);
}

void Handler::setupWifi(WiFiManager &network, NetworkServer &server,RGBLight &rgb) {
  network.Connect();

  if (!network.isConnected()) {
    DEBUG_WARN("Wrong credentials, starting AP for configuration...");
    server.StartAP();
    disableRGB(rgb);
    errorEncounteredRGB(rgb, SetupError::INVALID_CREDENTIALS);

    while (true) {
      server.HandleRequests();
      wait(10);
    }
  }
  disableRGB(rgb);
}

void Handler::setupSPS30(SPS30 &sps30, TwoWire &wire, RGBLight &rgb) {

  if (!sps30.begin(wire)) {
    DEBUG_WARN("Failed to find SPS30 sensor");
    errorEncounteredRGB(rgb, SetupError::SENSOR_INIT_FAILED);

    while (1)
      wait(10);
  }
  disableRGB(rgb);
}

void Handler::setupSHT41(SHT41Sensor &sht41, TwoWire &wire, RGBLight &rgb) {
  rgb.disable();

  if (!sht41.begin(wire)) {
    DEBUG_WARN("Failed to find SHT41 sensor");
    errorEncounteredRGB(rgb, SetupError::SENSOR_INIT_FAILED);

    while (1)
      wait(10);
  }

  disableRGB(rgb);
}

void Handler::enterDeepSleep(LEDStrip &strip, SegmentDisplay &segmentDisplay,
                             bool useSIM) {
  deepSleepManager.enterDeepSleep(strip, segmentDisplay, useSIM);
}