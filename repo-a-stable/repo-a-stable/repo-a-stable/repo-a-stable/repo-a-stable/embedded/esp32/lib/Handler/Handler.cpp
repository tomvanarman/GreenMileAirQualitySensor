#include <DEBUG.h>
#include <Handler.h>

void Handler::setupSim7080(SIM7080 &sim7080, LEDStrip &strip) {
  sim7080.initialize();
  sim7080.startModem();
  sim7080.setupNetwork();

  if (!sim7080.ensureConnected()) {
    DEBUG_WARN("Failed to connect to network");
    strip.stopLoading();
    strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);

    while (1)
      sim7080.ensureConnected();
  }

  strip.stopLoading();
  strip.clear();
}

void Handler::setupCredentialManager(CredentialManager &credential_manager,
                                     NetworkServer &server, LEDStrip &strip,
                                     SegmentDisplay &segmentDisplay) {
  credential_manager.LoadCredentials();

  if (!credential_manager.ValidateCredentials()) {
    DEBUG_WARN(
        "Invalid or missing credentials, starting AP for configuration...");
    server.StartAP();

    strip.stopLoading();
    strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);

    segmentDisplay.start();
    segmentDisplay.setIPAddress("192.168.4.1");

    while (true) {
      server.HandleRequests();
      wait(10);
    }
  }
}

void Handler::setupWifi(WiFiManager &network, NetworkServer &server,
                        LEDStrip &strip, SegmentDisplay &segmentDisplay) {
  network.Connect();

  if (!network.isConnected()) {
    DEBUG_WARN("Wrong credentials, starting AP for configuration...");
    server.StartAP();

    strip.stopLoading();
    strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);

    segmentDisplay.start();
    segmentDisplay.setIPAddress("192.168.4.1");

    while (true) {
      server.HandleRequests();
      wait(10);
    }
  }
}

void Handler::setupSPS30(SPS30 &sps30, TwoWire &wire, LEDStrip &strip) {
  strip.startLoading(CRGB::DeepSkyBlue, LEDStrip::_loadingModeType::BREATHING);

  if (!sps30.begin(wire)) {
    DEBUG_WARN("Failed to find SPS30 sensor");
    strip.stopLoading();
    strip.startLoading(CRGB::DeepSkyBlue, LEDStrip::_loadingModeType::BLINKING);

    while (1)
      wait(10);
  }

  strip.stopLoading();
}

void Handler::setupSHT41(SHT41Sensor &sht41, TwoWire &wire, LEDStrip &strip) {
  strip.startLoading(CRGB::DarkBlue, LEDStrip::_loadingModeType::BREATHING);

  if (!sht41.begin(wire)) {
    DEBUG_WARN("Failed to find SHT41 sensor");
    strip.stopLoading();
    strip.startLoading(CRGB::DarkBlue, LEDStrip::_loadingModeType::BLINKING);

    while (1)
      wait(10);
  }

  strip.stopLoading();
}

void Handler::enterDeepSleep(LEDStrip &strip, SegmentDisplay &segmentDisplay,
                             bool useSIM) {
  deepSleepManager.enterDeepSleep(strip, segmentDisplay, useSIM);
}