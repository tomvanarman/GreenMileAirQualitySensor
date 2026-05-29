# ESP32 Deep Sleep Power Optimization Test: Marineterrein USB-C + SIM7080

## Table of Contents

- [1. Purpose](#1-purpose)
- [2. Test Configuration](#2-test-configuration)
- [3. Firmware Changes Under Test](#3-firmware-changes-under-test)
- [4. Test Result](#4-test-result)
- [5. Network and Power Source Result](#5-network-and-power-source-result)
- [6. I2C Noise Observation](#6-i2c-noise-observation)
- [7. Second Clean Cycle Observation](#7-second-clean-cycle-observation)
- [8. Deep Sleep Shutdown Result](#8-deep-sleep-shutdown-result)
- [9. Remaining Validation](#9-remaining-validation)
- [10. Conclusion](#10-conclusion)

## 1. Purpose

Validate the firmware changes made to reduce power use before entering ESP32 deep sleep. This was a
SIM7080 field-style workflow test at the Marineterrein. WiFi was attempted first by the firmware, but
the successful network path in this run was SIM7080.

This test also records that the SIM7080 flow works from laptop USB-C power. Earlier tests suggested
that Otii power might be required for stable SIM7080 behavior, but this run shows that the important
fix was the modem power and startup handling in firmware, not Otii as the power source.

## 2. Test Configuration

- Test location: Marineterrein.
- Date and time: Friday, 22 May 2026, around `12:45`.
- Power source: laptop USB-C.
- UART baud rate: `115200`.
- WiFi SSID attempted by firmware: `Marnix's Galaxy S22 Ultra`.
- Successful network path: SIM7080 using MQTT.
- SIM registration result: roaming, GPRS connected.
- Sensor I2C pins:
    - SDA: `GPIO8`
    - SCL: `GPIO9`
- Sensors:
    - SPS30 particulate matter sensor at `0x69`.
    - SHT41 temperature and humidity sensor at `0x44`.
- Deep sleep target: next quarter-hour wake window.
- Current measurement: not measured during this run. This must be validated later at home with a
  current meter or Otii.

## 3. Firmware Changes Under Test

The deep sleep flow was extended so the firmware now shuts down peripherals before calling
`esp_deep_sleep_start()`.

### SPS30 sensor sleep

The SPS30 datasheet provides a sensor sleep command. The firmware now sends this command instead of
only stopping measurement mode.

```cpp
#define SPS30_CMD_SLEEP 0x1001
#define SPS30_CMD_WAKE_UP 0x1103
```

```cpp
bool SPS30::sleep()
{
    if (!_wire)
        return false;

    if (_measuring)
    {
        stopMeasurement();
        wait(20);
    }

    bool slept = sendCommand(SPS30_CMD_SLEEP);
    if (slept)
    {
        _initialized = false;
        _measuring = false;
    }
    return slept;
}
```

The wake-up command is sent before sensor probing so the device can recover after the previous cycle
put the SPS30 into sleep mode.

```cpp
bool SPS30::wakeUp(TwoWire &wire)
{
    _wire = &wire;

    sendCommand(SPS30_CMD_WAKE_UP);
    wait(5);

    bool woke = sendCommand(SPS30_CMD_WAKE_UP);
    wait(5);
    return woke;
}
```

### Deep sleep peripheral shutdown

The deep sleep manager now clears outputs, sleeps the SPS30, releases the sensor I2C bus, shuts down
wireless radios, and asks the SIM7080 driver to disable modem rails.

```cpp
void DeepSleepManager::preparePeripheralsForDeepSleep(
    LEDStrip &strip, SegmentDisplay &segmentDisplay, SPS30 &sps30,
    TwoWire &sensorWire, SIM7080 &sim7080, int sensorSdaPin,
    int sensorSclPin, bool useSIM) {
  DEBUG_SECTION("Deep Sleep Peripheral Shutdown");

  strip.clear();
  segmentDisplay.clearDisplay();

  if (sps30.isInitialized()) {
    if (sps30.sleep()) {
      DEBUG_OK("SPS30 entered sensor sleep mode");
    } else {
      DEBUG_WARN("SPS30 sleep command failed");
    }
  }
  sps30.resetState();

  sensorWire.end();

  pinMode(sensorSdaPin, INPUT);
  pinMode(sensorSclPin, INPUT);

  if (!useSIM) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    esp_wifi_stop();
  }

  btStop();
  esp_bt_controller_disable();

  sim7080.shutdownForDeepSleep();
}
```

### SIM7080 modem rail shutdown

The SIM7080 driver now has an explicit deep sleep shutdown path. It disconnects the modem when it
was started, ends the UART, releases modem pins, and disables the AXP2101 rails used by the modem.

```cpp
void SIM7080::shutdownForDeepSleep() {
    DEBUG_SECTION("SIM7080 Deep Sleep Shutdown");

    if (!_pmu.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        DEBUG_WARN("PMU unavailable, modem rails could not be disabled");
        return;
    }

    if (modemStarted_) {
        _modem.sendAT("+SMDISC");
        _modem.waitResponse(2000L);

        _modem.gprsDisconnect();

        _modem.sendAT("+CFUN=0");
        _modem.waitResponse(10000L);

        _modem.poweroff();
        _modem.waitResponse(5000L);
    }

    Serial1.end();
    modemStarted_ = false;

    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
    digitalWrite(BOARD_MODEM_DTR_PIN, LOW);
    pinMode(BOARD_MODEM_PWR_PIN, INPUT);
    pinMode(BOARD_MODEM_DTR_PIN, INPUT);
    pinMode(BOARD_MODEM_RXD_PIN, INPUT);
    pinMode(BOARD_MODEM_TXD_PIN, INPUT);

    DEBUG_INFO("Modem rails shutting down; ESP32 will enter deep sleep next");
    Serial.flush();
    wait(50);

    _pmu.disableDC3();
    _pmu.disableBLDO1();
    _pmu.disableVbusVoltageMeasure();
    _pmu.disableBattVoltageMeasure();
    _pmu.disableSystemVoltageMeasure();
    _pmu.disableBattDetection();

    DEBUG_OK("SIM7080 modem rails disabled for deep sleep");
}
```

## 4. Test Result

The device completed the full SIM7080 workflow successfully, despite heavy I2C startup noise.

The firmware:

- Started from laptop USB-C power.
- Tried WiFi first as part of the current firmware flow.
- Used SIM7080 successfully after WiFi timeout.
- Initialized the power chip.
- Started the SIM7080 modem.
- Registered on the network in roaming mode.
- Connected GPRS.
- Synchronized system time from the modem.
- Eventually recovered the I2C sensors.
- Sent SPS30 data over MQTT.
- Sent SHT41 data over MQTT.
- Entered the deep sleep flow.
- Put the SPS30 into sensor sleep mode.
- Disabled SIM7080 modem rails.
- Printed the final ESP32 deep sleep marker.

## 5. Network and Power Source Result

The SIM7080 worked while the board was powered from laptop USB-C.

```text
==================================================
Start Modem
==================================================

[INFO] Waiting for modem AT response...
[WARN] No AT response from modem
[WARN] No AT response from modem
[OK] Modem started!
```

```text
==================================================
Registering to Network
==================================================

[INFO] Waiting for network registration...
Network register info: : Registered, roaming.
GPRS status: : connected
[OK] Network registration completed!
```

This means Otii was not the key solution for SIM7080 stability. The tested firmware changes around
PMU setup, modem startup, and modem rail handling are what made the SIM7080 flow work reliably enough
for this run.

## 6. I2C Noise Observation

I2C noise is still present and remains the main reliability issue after the deep sleep work.

At startup, the sensor bus reported `SCL` low:

```text
==================================================
Sensor I2C Bus Recovery
==================================================

SDA idle : HIGH
SCL idle : LOW
```

The first known device probe found the SPS30 once, but not the SHT41:

```text
==================================================
I2C Known Device Probe
==================================================

[WARN] SHT41 missing at 0X44, error 5
[OK] SPS30 found at 0X69
```

The waiting loop then showed inconsistent results. Sometimes one sensor responded, sometimes neither
sensor responded.

```text
[WARN] Sensor I2C wait attempt 1 SHT41=missing SPS30=OK
[WARN] Sensor I2C wait attempt 2 SHT41=missing SPS30=missing
[WARN] Sensor I2C wait attempt 3 SHT41=OK SPS30=missing
[WARN] Sensor I2C wait attempt 4 SHT41=missing SPS30=missing
```

After the wait timeout, SPS30 initialization failed many times before eventually recovering.

```text
[WARN] Sensor I2C devices did not both respond before timeout
[WARN] SPS30 init attempt 1 failed
[WARN] SPS30 initialization attempt batch failed
[WARN] SPS30 not found yet, retrying
```

During the loop, both sensors continued to show symptoms of bus instability:

```text
[WARN] SPS30 read failed after retries
[WARN] Failed to read from sps30 sensor
[WARN] SHT41 not initialized, retrying setup
[WARN] SHT41 init attempt 1 failed
[WARN] SHT41 initialization attempt batch failed
```

Despite this, the retry behavior eventually recovered both sensors and the cycle finished.

## 7. Second Clean Cycle Observation

A later cycle behaved much better and looked like a clean golden path. In that run, both sensors were
found immediately, the SPS30 initialized and measured normally, the SHT41 initialized successfully
during setup, both MQTT payloads were published on the first attempt, and the firmware entered the
deep sleep shutdown flow without long sensor retries.

The exact reason why the second cycle worked so cleanly is not known yet. It may be related to sensor
warm-up state, bus state after the previous recovery, timing, cabling/contact stability, or another
hardware condition. For now, this should be recorded as an observation rather than treated as a
confirmed fix for the I2C noise.

## 8. Deep Sleep Shutdown Result

The final successful part of the run shows the device sending both MQTT payloads and entering deep
sleep.

```text
[INFO] Successfully sent SPS30 data
[OK] SHT41 initialized successfully
[INFO] Successfully sent SHT41 data
[OK] All data sent successfully, entering deep sleep
```

Deep sleep scheduling worked and targeted the next quarter-hour.

```text
==================================================
Deep Sleep
==================================================

[INFO] Entering deep sleep until next quarter
Current time : Fri May 22 12:45:44 2026

Next wake time : Fri May 22 13:00:00 2026

Sleep duration (seconds) : 856
```

The peripheral shutdown logs also completed.

```text
==================================================
Deep Sleep Peripheral Shutdown
==================================================

[OK] SPS30 entered sensor sleep mode

==================================================
SIM7080 Deep Sleep Shutdown
==================================================

[INFO] Modem rails shutting down; ESP32 will enter deep sleep next
[OK] SIM7080 modem rails disabled for deep sleep
[INFO] ESP32 entering deep sleep now
```

This confirms that the firmware reached the expected final shutdown point. The actual deep sleep
current still needs to be measured separately.

## 9. Remaining Validation

The tested firmware is good enough to merge to `develop` based on functional behavior. The feature
branch does not need to be deleted yet, because later home testing still needs to measure the actual
deep sleep current in mA.

Remaining work:

- Measure current after the final `ESP32 entering deep sleep now` message.
- Confirm whether the current is lower than the earlier `80-100 mA` result.
- Let the device wake at the next quarter-hour and confirm a second cycle starts normally.
- Investigate the remaining I2C noise, especially the `SCL idle : LOW` startup state and the
  inconsistent SHT41/SPS30 probe results.
- Compare noisy and clean cycles to find why the second test behaved perfectly.

## 10. Conclusion

This test is successful for firmware behavior. The device completed the whole workflow on USB-C
laptop power, used SIM7080 successfully, published both sensor payloads, and reached the final deep
sleep marker after shutting down the SPS30 and SIM7080 rails.

The main lesson is that Otii was not the key fix for SIM7080 stability. The code changes around the
SIM7080 PMU setup and shutdown path are what made the modem flow work on a normal USB-C power source.

The remaining issue is I2C noise. It did not block this cycle permanently, but it caused a long
recovery time and many failed sensor probes/reads before the firmware eventually recovered.
A second cycle appeared to work perfectly, but the cause of that improvement is still unknown.
