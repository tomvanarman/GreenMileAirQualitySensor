# SIM7080G Test: BLDO2 GNSS Antenna Rail

## Purpose

Determine whether the BLDO2 rail is needed for cellular bring-up and whether it
is safe to touch during the SIM7080G tests.

## Firmware Context

Early `catm` diagnostics printed detailed PMU steps around BLDO2 handling.

Observed last-known lines included:

```text
PMU: enable BLDO2 GNSS antenna rail at 3300 mV
PMU: keep BLDO2 GNSS antenna rail off
```

## Expected Result

For cellular-only tests, BLDO2 should not be required. BLDO2 is the GNSS antenna
rail, not the LTE modem supply.

## Observed Result

- Touching BLDO2 caused shutdown or hang behavior during early tests.
- Cellular tests do not need GNSS antenna power.

## Conclusion

BLDO2 is skipped entirely in the standalone cellular bring-up firmware. Revisit
this rail only when GNSS testing is required.
