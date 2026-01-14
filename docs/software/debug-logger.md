# DEBUG Logger

## Overview

The `DEBUG.h` library is a lightweight debugging utility for Arduino/ESP32 projects that provides
conditional compilation of debug output. It allows developers to easily add debug logging throughout
their code that can be globally enabled or disabled without removing debug statements from the
source code.

## Features

- **Global Debug Toggle**: Enable or disable all debug output with a single macro
- **Zero Runtime Overhead**: When disabled, debug statements are completely removed during
  compilation
- **Multiple Output Formats**: Support for simple prints, line prints, and formatted printf-style
  output
- **Arduino Compatible**: Designed specifically for Arduino/ESP32 Serial output

## Configuration

### Debug Toggle

The library uses a global `DEBUG` macro to control whether debug output is enabled:

```cpp
#define DEBUG 1  // Enable debug output
#define DEBUG 0  // Disable debug output
```

When `DEBUG` is set to `1`, all debug macros will output to the Serial port. When set to `0`, all
debug macros become empty and are removed during compilation, resulting in no performance overhead.

## Usage

### Macros

#### DEBUG_LOG(x)

```cpp
DEBUG_LOG(x)
```

Prints the specified value or string without adding a newline character.

**Parameters:**

- `x` - The value, variable, or string to print

**Example:**

```cpp
DEBUG_LOG("Temperature: ");
DEBUG_LOG(temperature);
DEBUG_LOG("°C");
// Output: Temperature: 25.5°C
```

#### DEBUG_LOG_LN(x)

```cpp
DEBUG_LOG_LN(x)
```

Prints the specified value or string and adds a newline character at the end.

**Parameters:**

- `x` - The value, variable, or string to print

**Example:**

```cpp
DEBUG_LOG_LN("System initialized");
DEBUG_LOG_LN("WiFi connecting...");
// Output:
// System initialized
// WiFi connecting...
```

#### DEBUG_PRINTF(fmt, ...)

```cpp
DEBUG_PRINTF(fmt, ...)
```

Prints formatted output using printf-style formatting.

**Parameters:**

- `fmt` - Format string with placeholders
- `...` - Variable arguments to fill the placeholders

**Example:**

```cpp
DEBUG_PRINTF("Sensor %d: %.2f°C\n", sensorId, temperature);
DEBUG_PRINTF("Network Status: %s, IP: %s\n",
             WiFi.isConnected() ? "Connected" : "Disconnected",
             WiFi.localIP().toString().c_str());
// Output: Sensor 1: 25.50°C
// Output: Network Status: Connected, IP: 192.168.1.100
```

### Production vs Development

```cpp
// In development - enable debug output
#define DEBUG 1
#include "DEBUG.h"

void developmentCode() {
    DEBUG_LOG_LN("This will appear in development");
    performOperation();
    DEBUG_LOG_LN("Operation completed");
}

// In production - disable debug output
#define DEBUG 0
#include "DEBUG.h"

void productionCode() {
    DEBUG_LOG_LN("This will NOT appear in production");
    performOperation(); // Only this line executes
    DEBUG_LOG_LN("This will also NOT appear in production");
}
```
