# ESP32 Home Full-Cycle Test: WiFi Fallback + SIM7080 MQTT

## Table of Contents

- [1. Purpose](#1-purpose)
- [2. Test Moment](#2-test-moment)
- [3. Test Setup](#3-test-setup)
- [4. Otii Power Summary](#4-otii-power-summary)
- [5. Cycle Result](#5-cycle-result)
- [6. Network Startup](#6-network-startup)
- [7. Sensor Startup](#7-sensor-startup)
- [8. SPS30 MQTT Publish](#8-sps30-mqtt-publish)
- [9. SHT41 MQTT Publish](#9-sht41-mqtt-publish)
- [10. Deep Sleep Validation](#10-deep-sleep-validation)
- [11. Conclusion](#11-conclusion)

## 1. Purpose

Document a home test of the current product firmware completing one full operational cycle. The test
validates that the device can recover from an unavailable WiFi connection, use the SIM7080 modem
instead, collect SPS30 and SHT41 measurements, publish both payloads over MQTT, and enter deep sleep
until the next quarter-hour wake window.

## 2. Test Moment

- Location: home test setup.
- Date and time: Tuesday, 19 May 2026, around `18:03-18:06`.
- Firmware behavior: current product cycle with WiFi first and SIM7080 fallback.
- Otii capture duration: `1m 34.520s`.
- Deep sleep scheduled from `18:05:26` to `18:15:00`.

## 3. Test Setup

- Otii connected as the main power source.
- Main voltage setpoint: `3.8 V`.
- Over-current protection: `2 A`.
- Digital voltage: `3.3 V`.
- UART baud rate: `115200`.
- WiFi SSID attempted: `Marnix's Galaxy S22 Ultra`.
- SIM7080 used as fallback network path.
- Sensor I2C pins:
    - SDA: `GPIO8`
    - SCL: `GPIO9`
- Sensors used:
    - SPS30 particulate matter sensor.
    - SHT41 temperature and humidity sensor.

## 4. Otii Power Summary

The Otii profile showed the full active cycle, including modem startup, network registration, sensor
retry behavior, MQTT publish bursts, and the transition toward deep sleep.

Captured summary values:

- Main current max: `346 mA`.
- Main current average: `121 mA`.
- Main current minimum: `-1.57 mA`.
- Charge used: `3.17 mAh`.
- Main voltage max: `3.86 V`.
- Main voltage average: `3.78 V`.
- Main voltage minimum: `0.0 V`.
- Main power max: `1.32 W`.
- Main power average: `460 mW`.
- Energy used: `12.1 mWh`.

## 5. Cycle Result

The complete cycle finished successfully.

The device:

- Tried to connect to WiFi.
- Timed out because WiFi credentials were unavailable or invalid.
- Fell back to the SIM7080 modem.
- Initialized the power chip.
- Started the modem after two missing AT responses.
- Registered to the LTE network in roaming mode.
- Connected GPRS.
- Synchronized system time from the SIM7080.
- Recovered the I2C bus enough for both sensors to respond.
- Published valid SPS30 data over MQTT.
- Retried SHT41 initialization until it recovered.
- Published valid SHT41 data over MQTT.
- Entered deep sleep until the next quarter-hour wake time.

## 6. Network Startup

WiFi was attempted first and failed, after which the firmware correctly switched to the SIM7080
path.

```text
SSID : Marnix's Galaxy S22 Ultra
[INFO] Connecting to WiFi...
[ERROR] Connection timeout! Invalid credentials?
[WARN] WiFi unavailable, falling back to SIM7080
[INFO] Disconnected from WiFi
```

The modem startup showed two initial missing AT responses, then recovered and continued.

```text
[INFO] Waiting for modem AT response...
[WARN] No AT response from modem
[WARN] No AT response from modem
[OK] Modem started!
```

Network registration and time sync completed successfully.

```text
Network register info: : Registered, roaming.
GPRS status: : connected
[OK] Network registration completed!
[OK] System time synchronized from SIM7080
```

## 7. Sensor Startup

The I2C bus was not fully idle at startup because `SCL` was low. Both known device probes initially
failed, but the waiting loop recovered and both sensors eventually responded.

```text
SDA idle : HIGH
SCL idle : LOW

[WARN] SHT41 missing at 0X44, error 2
[WARN] SPS30 missing at 0X69, error 2

[WARN] Sensor I2C wait attempt 1 SHT41=missing SPS30=missing
[WARN] Sensor I2C wait attempt 2 SHT41=missing SPS30=missing
[OK] SHT41 and SPS30 are responding on I2C
```

The SPS30 needed one failed initialization batch before it warmed up and started measuring. The
SHT41 was slower to recover and continued retrying during the main loop.

```text
[WARN] SPS30 init attempt 1 failed
[WARN] SPS30 initialization attempt batch failed
[WARN] SPS30 not found yet, retrying
[INFO] SPS30 warming up (5s)...
[OK] SPS30 initialized and measuring

[WARN] SHT41 init attempt 1 failed
[WARN] SHT41 init attempt 2 failed
[WARN] SHT41 init attempt 3 failed
[WARN] SHT41 initialization attempt batch failed
[WARN] SHT41 not ready during setup, continuing with loop retries
```

## 8. SPS30 MQTT Publish

The first SPS30 reads failed while the sensor was settling, but a valid payload was eventually
produced and published.

```json
{
    "mc_1p0": 0.003144,
    "mc_2p0": 0.003325,
    "mc_4p0": 0.003325,
    "mc_10p0": 0.003325,
    "nc_0p5": 0.021657,
    "nc_1p0": 0.024991,
    "nc_2p5": 0.025091,
    "nc_4p0": 0.025098,
    "nc_10p0": 0.025101,
    "typical_particle_size": 0.417636,
    "time_unix": 1779206707
}
```

MQTT publish result:

```text
MQTT topic : greenmile/hb1xbemtndayjlnbgpslb4cq/sps30/data
[INFO] Connecting SIM MQTT client...
[OK] SIM MQTT connected
[OK] SIM MQTT published
[INFO] Successfully sent SPS30 data
```

## 9. SHT41 MQTT Publish

The SHT41 repeatedly failed initialization attempts during the loop. Several low-level Wire errors
were observed, including `Error -1` and `Error 263`. Despite this, the retry logic eventually
initialized the sensor successfully.

```text
[WARN] SHT41 not initialized, retrying setup
[WARN] SHT41 init attempt 1 failed
[WARN] SHT41 initialization attempt batch failed

[E][Wire.cpp:513] requestFrom(): i2cRead returned Error -1
[E][Wire.cpp:513] requestFrom(): i2cRead returned Error 263

[OK] SHT41 initialized successfully
```

Successful SHT41 payload:

```json
{
    "temperature": 24.88785,
    "humidity": 58.58572,
    "time_unix": 1779206725
}
```

MQTT publish result:

```text
MQTT topic : greenmile/hb1xbemtndayjlnbgpslb4cq/sht41/data
[INFO] Connecting SIM MQTT client...
[OK] SIM MQTT connected
[OK] SIM MQTT published
[INFO] Successfully sent SHT41 data
```

## 10. Deep Sleep Validation

After both sensor payloads were sent, the firmware entered deep sleep and calculated the next wake
moment correctly.

```text
[OK] All data sent successfully, entering deep sleep

[INFO] Entering deep sleep until next quarter
Current time : Tue May 19 18:05:26 2026

Next wake time : Tue May 19 18:15:00 2026

Sleep duration (seconds) : 574
```

This confirms that quarter-hour scheduling still works after WiFi fallback, SIM7080 MQTT
transmission, sensor retries, and time synchronization from the modem.

## 11. Conclusion

This home test is successful and documents a complete current-product cycle.

The most important validation point is that the product can complete its full measurement and upload
flow even when WiFi fails and the I2C sensors are not ready immediately at boot. The SPS30 and SHT41
both recovered, both MQTT payloads were published through the SIM7080 path, and the device entered
deep sleep until the next scheduled quarter-hour.

Remaining attention point:

- SHT41 I2C startup remains noisy and can require many retries before recovery. The current retry
  behavior is effective, but the repeated `Wire.cpp` `requestFrom()` errors show that the bus
  stability issue is still present. If this noise can be prevented or fixed, the firmware cycle can
  likely complete faster because it will spend less time retrying sensor initialization and reads.
- Power consumption after entering deep sleep is still too high. During this test, the current
  remained around `80-100 mA` after deep sleep entry. This means the battery will drain quickly and
  will need to be changed sooner than intended. Deep sleep power optimization should be handled
  after the firmware behavior is completed.
