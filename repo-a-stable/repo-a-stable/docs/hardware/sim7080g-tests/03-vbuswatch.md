# SIM7080G Test: USB-C and VBUS Watch

## Purpose

Check whether plugging in USB-C/VBUS disturbs the board power path while the
board is powered from Otii/Qoitech. This test does not enable the SIM7080G modem
rail and does not run SIM logic.

## Firmware

PlatformIO environment:

```text
vbuswatch
```

Upload command:

```powershell
cd embedded/testsim
.\upload-monitor.cmd vbuswatch COM3
```

## What The Firmware Does

- Initializes the AXP2101 PMU.
- Disables battery charging for Otii/bench-source testing.
- Enables PMU voltage measurements.
- Toggles the PMU CHG LED while the firmware is alive.
- Prints VBUS, battery, system voltage, and charge state when serial is
  available.

## Expected Result

- With Otii-only power, the CHG LED keeps toggling.
- When USB-C is plugged in, the board should remain alive.
- A drop in Otii current alone does not prove shutdown because the PMU may switch
  the board load to USB/VBUS.

## Observed Result

- Powered from Otii at `3.75 V`, current was around `40 mA`.
- After USB-C was connected, Otii current dropped sharply, at one point to the
  microamp range.
- Opening a serial monitor caused the LED to pause briefly and then start again,
  likely due to USB serial reset/re-enumeration.
- The SHT41 rail LED was also observed turning off when USB-C was connected in
  some tests.

## Conclusion

USB-C/VBUS is currently a test disturbance. For modem tests, prefer Otii power
with USB data/debug that does not feed 5 V/VBUS into the board.
