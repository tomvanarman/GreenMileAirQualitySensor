# ESP32 Firmware Behavior

This page documents the current production behavior of the ESP32 climate box firmware. It focuses on
the runtime flow that matters when testing WiFi, MQTT, SIM fallback, sensor readings, and status
indicators.

## Startup Flow

On boot the firmware:

1. Starts serial logging at `115200` baud.
2. Configures the local timezone for Central Europe.
3. Initializes the RGB indicator.
4. Loads stored credentials from non-volatile storage.
5. Creates MQTT publish topics from the configured device ID.
6. Tries WiFi first when WiFi credentials are present.
7. Falls back to the SIM7080G modem when WiFi is unavailable or not configured.
8. Initializes the sensor I2C bus and waits for known sensors.
9. Reads and sends one measurement cycle before entering deep sleep.

The device credentials are still required for both WiFi and SIM operation, because they are used to
identify the device and build the MQTT topic names.

## Network Behavior

### WiFi First

If WiFi credentials are available, the firmware attempts to connect using `WiFiManager`. When the
connection succeeds:

- `useSIM` stays `false`.
- NTP time sync runs using the configured Central European timezone.
- The ESP MQTT client is initialized for `greenmile.tapp.city:1883`.
- Payloads are published over WiFi MQTT.

If WiFi credentials are present but the connection fails, the firmware logs a warning and switches
to SIM fallback.

### SIM Fallback

If WiFi credentials are missing or WiFi cannot connect, the firmware switches to SIM mode:

- `useSIM` becomes `true`.
- WiFi is disconnected and powered down.
- The SIM7080G modem is initialized and brought onto the network.
- The system time is synchronized from the SIM7080G timestamp when available.
- Payloads are published through the SIM7080G MQTT path.

The SIM fallback uses the same MQTT topic routing as WiFi. This is important: SHT41 and battery
payloads do not get sent to the SPS30 topic.

## MQTT Topics

The firmware publishes to one topic per sensor/data type. The device ID is read from stored
credentials.

| Payload type               | API path constant | MQTT topic                           |
| -------------------------- | ----------------- | ------------------------------------ |
| SPS30 particulate matter   | `SPS30path`       | `greenmile/<device_id>/sps30/data`   |
| SHT41 temperature/humidity | `SHT41path`       | `greenmile/<device_id>/sht41/data`   |
| Battery                    | `BatteryPath`     | `greenmile/<device_id>/battery/data` |

`sendPayload()` maps the path constant to the correct topic before publishing. Both WiFi MQTT and
SIM MQTT use this mapping.

## Measurement Lifecycle

The firmware is designed to collect one complete measurement cycle per wake:

1. Read SPS30.
2. Validate the SPS30 measurement.
3. Publish the SPS30 payload.
4. Read SHT41.
5. Validate the SHT41 measurement.
6. Publish the SHT41 payload.
7. Enter deep sleep when both payloads have been sent successfully.

The per-wake flags are:

- `sps30SentThisWake`
- `sht41SentThisWake`

The device only enters deep sleep after both are `true`. Battery payload support is present but
currently not part of the required wake cycle.

## Sensor Reliability

### I2C Bus Recovery

Before sensor setup, the firmware runs a sensor I2C recovery sequence:

- Ends the existing `WireSensors` bus.
- Checks whether SDA/SCL are idle high.
- Pulses SCL if SDA is stuck low.
- Reinitializes the sensor I2C bus.
- Probes the expected sensor addresses.

Expected sensor addresses:

| Sensor | I2C address |
| ------ | ----------- |
| SHT41  | `0x44`      |
| SPS30  | `0x69`      |

Current sensor bus settings:

- SDA: GPIO `8`
- SCL: GPIO `9`
- Clock: `50000` Hz
- Timeout: `1000` ms

### Validation

SPS30 readings are rejected when values are non-finite, negative, or outside expected upper bounds.
SHT41 readings are rejected when temperature or humidity is non-finite or outside the sensor's
expected range.

Rejected readings are not sent. The firmware logs a warning and retries in the loop.

## RGB Warning Behavior

The RGB indicator is initialized during setup. After initialization, every `DEBUG_WARN(...)` call
triggers a short red blink through `debugWarnIndicator()`.

This means warnings from the main firmware, sensor libraries, modem code, and network code are
visible both on serial output and on the RGB indicator.

Long-running setup failures can still use the explicit RGB error task in `Handler`, for example
invalid credentials or modem setup failure.

## PlatformIO Environment

The main firmware environment is:

```ini
[env:esp32s3usbotg]
board = esp32s3box
framework = arduino
monitor_speed = 115200
upload_speed = 921600
board_build.partitions = huge_app.csv
```

No fixed `upload_port` is configured. PlatformIO should auto-detect the device. If auto-detection
fails, pass a local upload port from the command line instead of committing a machine-specific port:

```powershell
pio run -e esp32s3usbotg -t upload --upload-port COM8
```

Build without uploading:

```powershell
pio run -e esp32s3usbotg
```

## Testing WiFi vs SIM

### Test WiFi MQTT

1. Store valid WiFi credentials and device credentials.
2. Boot the device.
3. Confirm serial logs show WiFi connected.
4. Confirm MQTT connects to `greenmile.tapp.city`.
5. Confirm SPS30 and SHT41 payloads are published to their own topics.
6. Confirm the device enters deep sleep after both payloads are sent.

### Test SIM MQTT Fallback

1. Store valid device credentials.
2. Remove WiFi credentials or make WiFi unavailable.
3. Boot the device with SIM and LTE antenna installed.
4. Confirm serial logs show SIM fallback.
5. Confirm the SIM7080G registers and connects.
6. Confirm payloads are published through `SIM7080::mqttPublish()`.
7. Confirm SPS30 and SHT41 payloads still use their own MQTT topics.

### Test Warning Indicator

Trigger a known warning condition, such as missing WiFi credentials or a missing sensor during
setup. The warning should appear in serial logs and the RGB LED should blink red briefly.
