# WiFi Manager

## Overview

The `WifiManager` class provides a simplified interface for WiFi operations on the ESP32. It handles
connecting, disconnecting, and automatic reconnection to a specified WiFi network using stored
credentials. The class includes timeout handling for connection attempts and provides debug logging
for connection status.

## Features

- **Automatic Connection Management**: Establishes and maintains WiFi connections
- **Connection Timeout Handling**: Uses a timeout for connection attempts to avoid indefinite
  blocking
- **Reconnection Support**: Provides automatic reconnection capabilities
- **Power Management**: Powers down WiFi radio when disconnecting to save energy
- **Debug Logging**: Uses ["DEBUG.h"](debug-logger.md) for detailed connection status output during
  development

## Dependencies

- **"WiFi.h"**: WiFi functionality for ESP32
- **"DEBUG.h"**: Custom debug logging (optional but recommended)

## Class Definition

```cpp
// WiFi connection manager for ESP32 devices.
//
// This class provides a simplified interface for WiFi operations on ESP32,
// including connection management, disconnection, and automatic reconnection.
class WifiManager
{
public:
    // Constructs a WiFi manager with network credentials.
    WifiManager(const String &ssid, const String &password);

    // Establishes connection to the configured WiFi network.
    void Connect();

    // Disconnects from WiFi network and powers down WiFi module.
    void Disconnect();

    // Attempts to reconnect to the configured WiFi network.
    void Reconnect();

private:
    const String ssid_;
    const String password_;

    const int connection_timeout_ = 10000; // 10 seconds timeout for connection attempts
};
```

## Constructor

### `WifiManager(const String& ssid, const String& password)`

Creates a new WifiManager instance with the specified network credentials.

**Parameters:**

- `ssid`: WiFi network name to connect to
- `password`: WiFi network password for authentication

**Example:**

```cpp
constexpr char kSsid[] = "network_name";
constexpr char kPassword[] = "network_password";

WifiManager network(kSsid, kPassword);
```

## Public Methods

### `void Connect()`

Establishes a connection to the configured WiFi network.

**Behavior:**

- Initiates WiFi connection using stored credentials
- Implements timeout for connection attempts
- Logs connection progress with dots (.) during attempt
- Displays network information upon successful connection
- Returns immediately if connection fails after timeout

**Example:**

```cpp
void setup()
{
    // Establish initial WiFi connection
    network.Connect();
}
```

### `void Disconnect()`

Gracefully disconnects from the WiFi network and powers down the WiFi radio.

**Behavior:**

- Closes existing WiFi connection
- Sets WiFi mode to `WIFI_OFF` to save power

**Example:**

```cpp
wifi.Disconnect();
```

### `void Reconnect()`

Attempts to reconnect to the configured WiFi network.

**Behavior:**

- Equivalent to calling `Connect()` when disconnected
- Implements same 10-second timeout as `Connect()`
- Used for recovering from connection loss
- Logs reconnection attempts and results

**Usage:**

```cpp
if (!WiFi.isConnected())
{
    wifi.Reconnect();
}
```

## Error Handling

The WifiManager implements several error handling strategies:

### Connection Timeout

- **Timeout Duration**: 10 seconds
- **Behavior**: Returns without connection if timeout exceeded
- **Recovery**: Call `Connect()` or `Reconnect()` again

### Network Unavailable

- **Detection**: Connection fails within timeout period
- **Logging**: Logs "Failed to connect to WiFi" or "Failed to reconnect to network"
- **Recovery**: Retry connection after delay

## Debug Information

The WifiManager provides detailed debug output when `DEBUG.h` is included:

```
Connecting to WiFi........
Network Status: Connected, Local IP: 192.168.1.100, Gateway IP: 192.168.1.1

Connection lost, attempting to reconnect.......
Successfully reconnected to network

Disconnected from WiFi network
```
