#pragma once
// Compatibility shim: LedControl was written for AVR and includes <avr/pgmspace.h>.
// On ESP32, the Arduino core ships pgmspace.h directly (no avr/ subdirectory).
#include <pgmspace.h>
