# SIM7080G Test: Full Cat-M Bring-Up

## Purpose

Run the actual SIM7080G modem and SIM internet test. This is the first test that
can prove the SIM card works.

## Firmware

PlatformIO environments:

```text
catm
nbiot
auto
```

Upload commands:

```powershell
cd embedded/testsim
.\upload-monitor.cmd catm COM3
.\upload-monitor.cmd nbiot COM3
.\upload-monitor.cmd auto COM3
```

## What The Firmware Tests

The full bring-up firmware attempts:

1. AXP2101 PMU setup.
2. BLDO1 level shifter rail at `3300 mV`.
3. DC3 modem rail at `3000 mV`.
4. SIM7080G UART setup.
5. SIM7080G PWRKEY boot sequence.
6. AT response check.
7. SIM card status check.
8. RAT/APN configuration.
9. LTE registration.
10. PDP bearer activation.
11. Network time query.
12. Ping test.
13. Plain HTTP GET test.
14. AT bridge for manual commands.

## Expected Success Output

Serial logs at `115200` baud should include:

```text
Modem responded to AT.
SIM is ready.
Registered on LTE network.
Bearer is active.
Ping test: PASS
HTTP test: PASS
SIM setup is working
```

## Observed Result

This full success path has not been confirmed yet. The current blocker is
getting reliable serial logs while the board is powered from the stable Otii
setup and not disturbed by USB-C/VBUS.

## Conclusion

This is the real SIM test. The PMU LED and SHT41 LED can only show that the board
stayed powered; they cannot prove SIM registration or internet access.
