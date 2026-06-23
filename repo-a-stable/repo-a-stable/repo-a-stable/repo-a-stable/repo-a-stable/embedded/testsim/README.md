# Testsim

Standalone LilyGO T-SIM7080G bring-up sketch. It powers the AXP2101 rails, starts the SIM7080G,
checks the SIM, configures APN/RAT, waits for LTE registration, activates `CNACT`, then runs a ping
and plain HTTP GET.

## Use

From this folder:

```powershell
pio run -e catm
pio run -e catm -t upload --upload-port COM8
pio device monitor -b 115200
```

On Windows, you can upload and then automatically open the monitor with:

```powershell
.\upload-monitor.cmd
```

If more than one COM device is connected, pass the port explicitly:

```powershell
.\upload-monitor.cmd catm COM8
```

Available environments:

- `catm`: default, uses LTE Cat-M.
- `nbiot`: use when your SIM/network only supports NB-IoT at the test location.
- `auto`: lets the modem use Cat-M or NB-IoT.
- `dc3test`: PMU-only diagnostic for the SIM7080G DC3 rail at 3000 mV.
- `dc3low`: same diagnostic with DC3 at 2700 mV.
- `bldo1dc3`: same diagnostic as `dc3test`, but also enables BLDO1 before DC3.
- `bldo1dc3hold`: BLDO1 + DC3 diagnostic that avoids PMU I2C reads after DC3 turns on.
- `bldo1dc3pins`: same as `bldo1dc3hold`, but holds modem `PWRKEY` and `DTR` low before DC3 turns
  on.
- `bldo1dc3pwm`: same as `bldo1dc3pins`, but forces DC3 PWM mode before DC3 turns on.
- `bldo1dc3nouvp`: same as `bldo1dc3pins`, but disables DC3 low-voltage PMIC turn-off before DC3
  turns on.
- `modemscan`: powers the modem rail safely, pulses PWRKEY, and scans common UART baud rates for
  `AT`.
- `modemscan33`: same as `modemscan`, but sets DC3 to 3300 mV.
- `modemscanswap`: same as `modemscan`, but swaps the ESP32 UART RX/TX pins.
- `modemscanbldo2`: same as `modemscan`, but also enables BLDO2.
- `modemkeyscan`: same power setup as `modemscan`, but tries multiple PWRKEY pulse timings and
  polarities.
- `vbuswatch`: PMU-only diagnostic for plugging/unplugging USB-C while powered from Otii or a
  battery source.
- `serialtest`: USB serial heartbeat only.
- `blinktest`: PMU CHG LED toggle only.

For the DC3 diagnostic, serial is optional. Battery charging is disabled so USB-C does not try to
charge an Otii/bench supply connected to the battery input. After upload, the CHG LED blinks at 1 Hz
before DC3 turns on, then blinks once per second if the board survives DC3. If the board powers off
at that point, the issue is on the DC3/modem power rail or the board supply path, not LTE
registration code.

The default APN is `iot.1nce.net`. To test another SIM, change `TESTSIM_APN` in `platformio.ini`.

## Hardware notes

- Insert the SIM before powering or resetting the board.
- Use a SIM that supports LTE Cat-M or NB-IoT; the SIM7080G does not support 2G, 3G, or normal 4G
  data.
- Connect the LTE antenna before testing.
- Serial monitor should be `115200` baud. For the AT bridge at the end, select `Both NL & CR`.
