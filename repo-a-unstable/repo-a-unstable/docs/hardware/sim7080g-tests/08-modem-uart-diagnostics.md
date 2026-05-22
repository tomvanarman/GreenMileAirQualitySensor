# SIM7080G Test: Modem UART Diagnostics

## Purpose

After the PMU rail sequence became stable, the full Cat-M test reached the modem
startup step but initially received no `AT` response. This test series checked
whether the modem was alive, whether the baud rate was wrong, whether UART pins
were swapped, whether BLDO2 was required, or whether PWRKEY timing was wrong.

## Firmware

PlatformIO environments:

```text
modemscan
modemscan33
modemscanswap
modemscanbldo2
modemkeyscan
```

Upload commands:

```powershell
cd embedded/testsim
.\upload-monitor.cmd modemscan COM3
.\upload-monitor.cmd modemscan33 COM3
.\upload-monitor.cmd modemscanswap COM3
.\upload-monitor.cmd modemscanbldo2 COM3
.\upload-monitor.cmd modemkeyscan COM3
```

## What The Firmware Tested

- `modemscan`: DC3 stable setup, PWRKEY pulse, and baud scan.
- `modemscan33`: same test with DC3 at `3300 mV`.
- `modemscanswap`: same test with ESP32 UART RX/TX swapped.
- `modemscanbldo2`: same test with BLDO2 enabled.
- `modemkeyscan`: multiple PWRKEY pulse timings and polarities.

The baud scan tested:

```text
115200
9600
19200
38400
57600
230400
460800
```

## Observed Result

Before the mechanical issue was found, all modem scan tests reported:

```text
no bytes
```

That ruled out these likely software causes:

- Wrong baud rate.
- UART RX/TX reversed in firmware.
- Missing BLDO2 for cellular UART response.
- Too-short or wrong-polarity PWRKEY pulse.
- DC3 at `3000 mV` versus `3300 mV`.

During inspection, a metal bolt near the SIM7080G/NB-IoT area was found touching
the board and getting hot. After the bolt was moved so it no longer made
contact, the modem responded immediately at `115200` baud:

```text
Passive baud scan before PWRKEY pulse.
Probe AT at 115200 baud, attempt 1
  no bytes
Probe AT at 115200 baud, attempt 2
AT
OK
  AT OK at 115200 baud
AT bridge ready at 115200 baud. Use Both NL & CR.
```

A red modem LED was also observed, sometimes blinking.

## Conclusion

The ESP32-to-SIM7080G UART configuration is correct:

- UART baud: `115200`.
- ESP32 modem RX pin: `BOARD_MODEM_RXD_PIN`.
- ESP32 modem TX pin: `BOARD_MODEM_TXD_PIN`.
- BLDO1 must be enabled for the level shifter.

The earlier no-byte failures were most likely caused by a partial short from the
metal bolt near the modem area. Metal hardware must not touch the SIM7080G/NB-IoT
area; use nylon hardware, washers, or insulation.
