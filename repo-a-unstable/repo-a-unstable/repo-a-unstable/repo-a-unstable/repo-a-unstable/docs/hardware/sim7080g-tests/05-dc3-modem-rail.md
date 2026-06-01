# SIM7080G Test: DC3 Modem Rail

## Purpose

Verify whether the AXP2101 DC3 rail can be enabled without killing the board.
DC3 is treated as the SIM7080G modem supply rail.

This test does not pulse SIM7080G PWRKEY and does not check AT response, SIM
state, registration, ping, or HTTP.

## Firmware

PlatformIO environments:

```text
dc3test
dc3low
```

Upload commands:

```powershell
cd embedded/testsim
.\upload-monitor.cmd dc3test COM3
.\upload-monitor.cmd dc3low COM3
```

## What The Firmware Does

- Initializes the AXP2101 PMU.
- Disables battery charging for Otii/bench-source testing.
- Blinks the PMU CHG LED at 1 Hz before DC3.
- Performs 3 fast CHG LED blinks.
- Enables DC3.
- Toggles the CHG LED once per second if firmware remains alive.

## Expected Result

After the 3 fast blinks:

- SHT41 LED stays on: the board power rail is still alive.
- SHT41 LED turns off: the board really lost sensor/system rail power.

## Observed Result

- Earlier serial output reached:

```text
PMU: set DC3 modem rail voltage to 3000 mV
PMU: DC3 voltage set OK
PMU: enable DC3 modem rail
PMU: DC3 enabled OK
```

- With Otii power, after the 3 fast blinks the SHT41 LED stayed on.

## Conclusion

DC3 enable does not currently appear to kill the whole board when powered from
Otii. This only proves modem rail power stability. It does not prove that the
modem has booted or that the SIM works.
