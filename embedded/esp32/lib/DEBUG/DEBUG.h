#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

// Toggle debug logging globally
#define DEBUG 1

#if DEBUG
#define DEBUG_LOG(x) Serial.print(x)
#define DEBUG_LOG_LN(x) Serial.println(x)
#define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)

// --- Structured logging helpers ---

// Section headers
#define DEBUG_SECTION(title)                                                       \
    do                                                                             \
    {                                                                              \
        Serial.println(F("\n==================================================")); \
        Serial.println(title);                                                     \
        Serial.println(F("==================================================\n")); \
    } while (0)

// API Header
#define DEBUG_SECTION_API(title)                                                   \
    do                                                                             \
    {                                                                              \
        Serial.println(F("\n++++++++++++++++++++++++++++++++++++++++++++++++++")); \
        Serial.println(title);                                                     \
        Serial.println(F("++++++++++++++++++++++++++++++++++++++++++++++++++\n")); \
    } while (0)

// Subsection or block
#define DEBUG_BLOCK(title)                                                         \
    do                                                                             \
    {                                                                              \
        Serial.println(F("\n--------------------------------------------------")); \
        Serial.println(title);                                                     \
        Serial.println(F("--------------------------------------------------"));   \
    } while (0)

// Step markers
#define DEBUG_STEP(step, msg) \
    Serial.println("[" + String(step) + "] " + String(msg))

// Success / failure
#define DEBUG_OK(msg) Serial.println("[OK] " + String(msg))
#define DEBUG_FAIL(msg) Serial.println("[ERROR] " + String(msg))
#define DEBUG_WARN(msg) Serial.println("[WARN] " + String(msg))
#define DEBUG_INFO(msg) Serial.println("[INFO] " + String(msg))

// Key-value aligned output
#define DEBUG_KV(key, value) \
    Serial.println(String(key) + " : " + String(value))

// Blank line
#define DEBUG_NEWLINE() Serial.println()

#else
#define DEBUG_LOG(x)
#define DEBUG_LOG_LN(x)
#define DEBUG_PRINTF(fmt, ...)

#define DEBUG_SECTION(title)
#define DEBUG_BLOCK(title)
#define DEBUG_STEP(step, msg)
#define DEBUG_OK(msg)
#define DEBUG_FAIL(msg)
#define DEBUG_WARN(msg)
#define DEBUG_INFO(msg)
#define DEBUG_KV(key, value)
#define DEBUG_NEWLINE()

#endif // DEBUG

#endif // DEBUG_H
