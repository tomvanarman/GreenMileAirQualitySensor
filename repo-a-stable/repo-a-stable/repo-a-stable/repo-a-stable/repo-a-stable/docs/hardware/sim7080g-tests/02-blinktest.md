# SIM7080G Test: PMU CHG LED

## Purpose

Verify that the ESP32-S3 can initialize the AXP2101 PMU over I2C and control the
PMU CHG LED. This provides a visual diagnostic path when serial output is not
reliable.

## Firmware

PlatformIO environment:

```text
blinktest
```

Upload command:

```powershell
cd embedded/testsim
.\upload-monitor.cmd blinktest COM3
```

## Expected Result

The PMU CHG LED toggles once per second.

## Observed Result

- PMU CHG LED control worked.
- This proved the ESP32-S3 can communicate with the AXP2101 PMU.

## Conclusion

The PMU LED is usable as a basic visual heartbeat for power and firmware
diagnostics.
