# SIM7080G Test: Serial Heartbeat

## Purpose

Verify that the ESP32-S3 can run uploaded firmware and print over USB serial.
This test does not touch the AXP2101 PMU, SIM7080G modem, SIM card, or network.

## Firmware

PlatformIO environment:

```text
serialtest
```

Upload command:

```powershell
cd embedded/testsim
.\upload-monitor.cmd serialtest COM3
```

## Expected Result

Serial monitor at `115200` baud should print:

```text
testsim serial heartbeat
```

once per second.

## Observed Result

- COM3 was confirmed capable of printing serial output.
- PlatformIO's automatic monitor path was unreliable during the investigation.
- The upload helper was changed to upload only and print manual monitor
  instructions instead of opening the monitor automatically.

## Conclusion

Firmware upload and USB serial output can work, but serial monitoring is not a
stable proof of modem behavior while the USB-C power path is disturbing the
board.
