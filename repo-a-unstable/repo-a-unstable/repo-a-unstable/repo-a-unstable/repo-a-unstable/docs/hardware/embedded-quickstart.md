
# Embedded Hardware – Quickstart

## Overview
This document summarizes the embedded hardware and software stack for the Climate Measuring Box (CMB).

## Toolchain
- **PlatformIO** (in VSCode): Professional embedded development, multi-platform, easy library and build management.

## Hardware Components
- **Microcontroller:** Lilygo T-SIM7080G (ESP32 + SIM7080G LTE)
- **Sensors:**
	- SPS30 (particulate matter) ([SPS30.md](SPS30.md))
	- SHT41 (temperature/humidity) ([SHT41.md](SHT41.md))
- **Wireless:** SIM7080G LTE module ([Ite-interface](../software/Ite-interface.md))
- **Battery:** LiFePo4 (safe, robust, long life) ([LiFePo4.md](LiFePo4.md))


## Frameworks & Libraries
- **Arduino framework** for embedded development

### Project Libraries (lib/)
- **ColorMap**: Color mapping utilities
- **CredentialManager**: Credential management for secure access
- **DEBUG**: Logging/debugging utilities (by Sebas)
- **Handler**: General event/logic handler
- **HelpMethod**: Helper methods for various tasks
- **HttpManager**: HTTP communication utilities
- **JsonBuilder**: JSON construction helpers
- **LEDStrip**: LED strip control
- **MqttClient**: Custom MQTT wrapper
- **NetworkServer**: ESP32 webserver for credentials/config
- **PushButton**: Push button handling
- **SegmentDisplay**: Segment display control
- **SHT41Sensor**: Custom SHT41 wrapper
- **SIM7080**: SIM7080G LTE module utilities
- **SPS30**: SPS30 sensor integration
- **TinyGSM**: LTE communication
- **utilities**: General utility functions
- **WifiManager**: WiFi management
- **XPowersLib**: Power management

### External Libraries (libdeps/)
- Adafruit SHT4x
- Adafruit BusIO
- Adafruit GFX
- Adafruit SSD1306
- Adafruit Unified Sensor
- ArduinoJson
- FastLED
- LedControl
- PubSubClient
