<!-- TOC -->
## Table of Content
1. [Code Improvements](#1-code-improvements) \
    1.1 [Overview](#11-overview)
2. [Modularize Arduino Code](#2-modularize-arduino-code)
3. [Old Code that is not clear](#3-old-code-that-is-not-clear) 
4. [Improved Code](#4-improved-code) \
  4.1 [main setup](#41-main-setup)\
  4.2 [Handler](#42-handler)
5. [Conclusion](#5-conclusion)
6. [References](#6-references)
<!-- /TOC -->

# 1. Code Improvements
## 1.1 Overview

The previous students had already had a code base which was functional. The problem was that the setup method was relatively crowded. This made it hard to have a clear overview on what is happening in that phase of the prototype. Therefore, I want to implement a handler file that can be used to set up all the different parts of the air pollution sensor box and makes the setup method more readable. In addition, it makes the code better expandable, with the ability to easily add it to the handler.

# 2. Modularize Arduino Code
There are possibilities to make C++ code more readable and to use modularisation to structure it more clearly. This will come in handy when you want to debug or understand the code. In general, the idea behind handlers is, when you want to have better security or, in this case, have a more complex construct, you use them, so the user only sees the cleaner version. [1] Each parts get put into modules that perform a specific task. [2]

# 3. Old Code that is not clear
```c++

void setup()
{
  Serial.begin(115200);

  Serial.println();
  Serial.println("====================================");
  Serial.println("Starting GreenMile Air Quality Sensor");

  wait(5000); // Give some time to open the serial monitor after reset

  DEBUG_SECTION("Setup");

  strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BREATHING);

  if (useSIM)
  {
    // ============================================================================
    // Setup SIM7080
    // ============================================================================
    sim7080.initialize();
    sim7080.startModem();
    sim7080.setupNetwork();
    if (!sim7080.ensureConnected())
    {
      DEBUG_WARN("Failed to connect to network");
      strip.stopLoading();
      strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);
      while (1)
        sim7080.ensureConnected();
    }
    strip.stopLoading();
    strip.clear();
  }
  else
  {
    credential_manager.LoadCredentials();

    if (!credential_manager.ValidateCredentials())
    {
      DEBUG_WARN("Invalid or missing credentials, starting AP for configuration...");
      server.StartAP();

      strip.stopLoading();
      strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);

      segmentDisplay.start();
      segmentDisplay.setIPAddress("192.168.4.1");

      // Wait indefinitely in AP mode until configured
      while (true)
      {
        server.HandleRequests();
        wait(10);
      }
    }

    //============================================================================
    // Setup WiFi
    //============================================================================
    network.Connect();

    if (!network.isConnected())
    {
      DEBUG_WARN("Wrong credentials, starting AP for configuration...");
      server.StartAP();

      strip.stopLoading();
      strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);

      segmentDisplay.start();
      segmentDisplay.setIPAddress("192.168.4.1");

      // Wait indefinitely in AP mode until configured
      while (true)
      {
        server.HandleRequests();
        wait(10);
      }
    }

    // Initialize NTP time synchronization
    initializeTime();
  }

  strip.stopLoading();
  strip.clear();

  // Initialize I2C communication
  WireSensors.begin(8, 9);      // SDA, SCL
  WireSensors.setClock(100000); // lock to 100 kHz for both SPS30 and SHT41
  WireSensors.setTimeOut(100);  // a bit more headroom for long reads

  //============================================================================
  // Setup SPS30
  //============================================================================
  strip.startLoading(CRGB::DeepSkyBlue, LEDStrip::_loadingModeType::BREATHING);
  if (!sps30.begin(WireSensors))
  {
    DEBUG_WARN("Failed to find SPS30 sensor");
    strip.stopLoading();
    strip.startLoading(CRGB::DeepSkyBlue, LEDStrip::_loadingModeType::BLINKING);
    while (1)
      wait(10);
  }
  strip.stopLoading();

  //============================================================================
  // Setup SHT41
  //============================================================================
  strip.startLoading(CRGB::DarkBlue, LEDStrip::_loadingModeType::BREATHING);
  if (!sht41.begin(WireSensors))
  {
    DEBUG_WARN("Failed to find SHT41 sensor");
    strip.stopLoading();
    strip.startLoading(CRGB::DarkBlue, LEDStrip::_loadingModeType::BLINKING);
    while (1)
      wait(10);
  }
  strip.stopLoading();
  strip.clear();

  // Initialize LUT for color mapping
  colorMap.InitLUT();

  // Initialize segment display
  segmentDisplay.start();
  // int batteryLevel = determineBatteryLevel();
  // segmentDisplay.setBattery(batteryLevel);
  // wait(3000);
  segmentDisplay.clearDisplay();
}
```

 # 4. Improved Code
## 4.1 main setup
```c++
void setup()
{
  Serial.begin(115200);

  Serial.println();
  Serial.println("====================================");
  Serial.println("Starting GreenMile Air Quality Sensor");

  wait(5000); // Give some time to open the serial monitor after reset

  DEBUG_SECTION("Setup");

  strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BREATHING);

  if (useSIM)
  {
    // ============================================================================
    // Setup SIM7080
    // ============================================================================
    handler.setupSim7080(sim7080, strip);
  }
  else
  {
    handler.setupCredentialManager(credential_manager, server, strip, segmentDisplay);

    //============================================================================
    // Setup WiFi
    //============================================================================
    handler.setupWifi(network, server, strip, segmentDisplay);

    // Initialize NTP time synchronization
    initializeTime();
  }

  strip.stopLoading();
  strip.clear();

  // Initialize I2C communication
  WireSensors.begin(8, 9);      // SDA, SCL
  WireSensors.setClock(100000); // lock to 100 kHz for both SPS30 and SHT41
  WireSensors.setTimeOut(100);  // a bit more headroom for long reads

  //============================================================================
  // Setup SPS30
  //============================================================================
  handler.setupSPS30(sps30, WireSensors, strip);

  //============================================================================
  // Setup SHT41
  //============================================================================
  handler.setupSHT41(sht41, WireSensors, strip);

  strip.clear();

  // Initialize LUT for color mapping
  colorMap.InitLUT();

  // Initialize segment display
  segmentDisplay.start();
  // int batteryLevel = determineBatteryLevel();
  // segmentDisplay.setBattery(batteryLevel);
  // wait(3000);
  segmentDisplay.clearDisplay();
}

```
## 4.2 Handler
```c++

#include <Handler.h>

void Handler::setupSim7080(SIM7080 &sim7080, LEDStrip &strip)
{
    sim7080.initialize();
    sim7080.startModem();
    sim7080.setupNetwork();
    if (!sim7080.ensureConnected())
    {
        DEBUG_WARN("Failed to connect to network");
        strip.stopLoading();
        strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);
        while (1)
            sim7080.ensureConnected();
    }
    strip.stopLoading();
    strip.clear();
}

void Handler::setupCredentialManager(CredentialManager &credential_manager, NetworkServer &server, LEDStrip &strip, SegmentDisplay &segmentDisplay)
{
    credential_manager.LoadCredentials();

    if (!credential_manager.ValidateCredentials())
    {
      DEBUG_WARN(
          "Invalid or missing credentials, starting AP for configuration...");
      server.StartAP();

      strip.stopLoading();
      strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);

      segmentDisplay.start();
      segmentDisplay.setIPAddress("192.168.4.1");

      while (true)
      {
        server.HandleRequests();
        wait(10);
      }
    }
}

void Handler::setupWifi(WiFiManager &network, NetworkServer &server, LEDStrip &strip, SegmentDisplay &segmentDisplay)
{
    network.Connect();

    if (!network.isConnected())
    {
      DEBUG_WARN("Wrong credentials, starting AP for configuration...");
      server.StartAP();

      strip.stopLoading();
      strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);

      segmentDisplay.start();
      segmentDisplay.setIPAddress("192.168.4.1");

      while (true)
      {
        server.HandleRequests();
        wait(10);
      }
    }
}

void Handler::setupSPS30(SPS30 &sps30, TwoWire &wire, LEDStrip &strip)
{
    strip.startLoading(CRGB::DeepSkyBlue, LEDStrip::_loadingModeType::BREATHING);
    if (!sps30.begin(wire))
    {
        DEBUG_WARN("Failed to find SPS30 sensor");
        strip.stopLoading();
        strip.startLoading(CRGB::DeepSkyBlue, LEDStrip::_loadingModeType::BLINKING);
        while (1)
            wait(10);
    }
    strip.stopLoading();
}

void Handler::setupSHT41(SHT41Sensor &sht41, TwoWire &wire, LEDStrip &strip)
{
    strip.startLoading(CRGB::DarkBlue, LEDStrip::_loadingModeType::BREATHING);
    if (!sht41.begin(wire))
    {
        DEBUG_WARN("Failed to find SHT41 sensor");
        strip.stopLoading();
        strip.startLoading(CRGB::DarkBlue, LEDStrip::_loadingModeType::BLINKING);
        while (1)
            wait(10);
    }
    strip.stopLoading();
}

```

 # 5. Conclusion

In summary, the code has been outsourced to be clearer and better structured, so that it will be easier to maintain and understand. This part helped me to understand modularisation in C++ and improve the group code base quality.

 # 6. References

> [1] What Are Handlers in Programming?, https://www.baeldung.com/cs/handlers-programming, accessed 30.03.2026 \
> [2] Module pattern, https://en.wikipedia.org/wiki/Module_pattern, accessed 30.03.2026
