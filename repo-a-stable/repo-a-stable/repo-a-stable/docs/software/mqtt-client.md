# MqttClient Class Documentation

## Overview

The `MqttClient` class provides a simplified interface for MQTT operations on ESP32 devices. It
handles connection management, message publishing, and automatic reconnection while integrating
seamlessly with the WifiManager for network connectivity management.

## Features

- **Simplified MQTT Interface**: Easy-to-use wrapper around PubSubClient library
- **WiFi Dependency Management**: Automatically checks WiFi connectivity before MQTT operations
- **Connection State Management**: Handles MQTT connection lifecycle and status checking
- **Message Publishing**: Publishes messages to MQTT topics with error handling
- **Automatic Reconnection**: Provides reconnection capabilities for network recovery
- **Debug Logging**: Comprehensive logging for connection status and message publishing
- **Keep-Alive Management**: Maintains MQTT connection through regular loop processing

## Dependencies

- **PubSubClient**: Arduino MQTT library for ESP32
- **WiFi**: ESP32 built-in WiFi functionality
- **WifiManager**: Custom WiFi management class
- **DEBUG.h**: Custom debug logging system

## Class Definition

```cpp
class MqttClient {
 public:
  MqttClient(const char* server, int port, WifiManager& wifi_manager);
  void Connect();
  void Disconnect();
  void Reconnect();
  void Publish(const char* topic, const String& payload);
  bool IsConnected();
  void Loop();

 private:
  const char* server_;        // MQTT broker hostname/IP
  int port_;                  // MQTT broker port
  WifiManager& wifi_manager_; // WiFi connectivity manager
  WiFiClient esp_client_;     // Underlying WiFi client
  PubSubClient client_;       // MQTT protocol implementation
};
```

## Constructor

### `MqttClient(const char* server, int port, WifiManager& wifi_manager)`

Creates a new MqttClient instance with the specified broker configuration.

**Parameters:**

- `server`: MQTT broker hostname or IP address (must remain valid for object lifetime)
- `port`: MQTT broker port number (typically 1883 for non-SSL, 8883 for SSL)
- `wifi_manager`: Reference to WiFi manager for connectivity checks

**Example:**

```cpp
WifiManager wifi("MyNetwork", "MyPassword");
MqttClient mqtt("broker.example.com", 1883, wifi);
```

**Important Notes:**

- The `server` pointer must remain valid for the entire lifetime of the MqttClient object
- The `wifi_manager` reference is used to check WiFi connectivity before MQTT operations

## Public Methods

### `void Connect()`

Establishes connection to the MQTT broker using a fixed client ID.

**Preconditions:**

- WiFi must be connected (`WiFi.isConnected()` returns true)

**Behavior:**

- Checks WiFi connectivity before attempting connection
- Sets MQTT server configuration
- Uses "ESP32Client" as the fixed client ID
- Logs connection attempts and results
- Gracefully handles already-connected state (no-op)
- Logs specific error codes on connection failure

**Usage:**

```cpp
wifi.Connect();  // Ensure WiFi is connected first
mqtt.Connect();  // Then connect to MQTT broker

if (mqtt.IsConnected()) {
    // MQTT connection successful
}
```

**Debug Output:**

```
Connecting MQTT server: broker.example.com on port: 1883
Attempting MQTT connection...
MQTT connected!
```

**Error Codes:** Common MQTT connection error codes:

- `-1`: Connection lost
- `-2`: Connect failed
- `-3`: Connection lost
- `-4`: Connection timeout
- `-5`: Connection refused, bad protocol
- `-6`: Connection refused, bad client ID

### `void Disconnect()`

Closes the MQTT connection gracefully.

**Behavior:**

- Only disconnects if currently connected (avoids unnecessary operations)
- Calls underlying PubSubClient disconnect method
- Logs disconnection status
- Safe to call when already disconnected

**Usage:**

```cpp
mqtt.Disconnect();
// MQTT connection is now closed
```

### `void Reconnect()`

Attempts to reconnect to the MQTT broker.

**Behavior:**

- Checks WiFi connectivity before attempting reconnection
- Delegates to `Connect()` method for actual connection logic
- Equivalent to calling `Connect()` when disconnected
- Used for recovering from connection loss

**Usage:**

```cpp
if (!mqtt.IsConnected()) {
    mqtt.Reconnect();
}
```

### `void Publish(const char* topic, const String& payload)`

Publishes a message to the specified MQTT topic.

**Parameters:**

- `topic`: MQTT topic string (null-terminated)
- `payload`: Message content to publish as Arduino String

**Behavior:**

- Checks connection status before attempting to publish
- Converts String payload to C-string for PubSubClient
- Captures and logs publish success/failure status
- Returns silently if not connected (logs attempt for debugging)

**Usage:**

```cpp
// Simple message publishing
mqtt.Publish("sensors/temperature", "25.6");
mqtt.Publish("device/status", "online");

// Publishing sensor data
String sensorData = "{\"temperature\":25.6,\"humidity\":60.2}";
mqtt.Publish("sensors/data", sensorData);
```

**Common Topics:**

- `sensors/temperature` - Temperature readings
- `sensors/humidity` - Humidity readings
- `device/status` - Device status updates
- `commands/response` - Command responses
- `telemetry/data` - General telemetry data

### `bool IsConnected()`

Returns the current MQTT connection status.

**Returns:**

- `true` if connected to MQTT broker
- `false` if disconnected or connection lost

**Usage:**

```cpp
if (mqtt.IsConnected()) {
    mqtt.Publish("sensors/data", data);
} else {
    mqtt.Reconnect();
}
```

### `void Loop()`

Processes incoming MQTT messages and maintains connection keep-alive.

**Behavior:**

- Calls underlying PubSubClient loop() method
- Processes incoming MQTT messages (subscriptions)
- Maintains MQTT keep-alive mechanism
- **Must be called regularly** for proper MQTT operation

**Usage:**

```cpp
void loop() {
    mqtt.Loop();  // Call regularly in main loop

    // Other application code
    delay(100);
}
```

**Critical Requirement:** This method must be called frequently (at least every few seconds) to:

- Process incoming messages
- Send keep-alive packets
- Detect connection failures
- Handle message acknowledgments

## Connection States and Error Handling

### MQTT Connection States

| State             | Description                      | IsConnected() | Action Required           |
| ----------------- | -------------------------------- | ------------- | ------------------------- |
| Connected         | Successfully connected to broker | `true`        | Normal operation          |
| Disconnected      | Not connected to broker          | `false`       | Call Connect()            |
| Connection Failed | Authentication or network error  | `false`       | Check credentials/network |
| Connection Lost   | Previously connected but lost    | `false`       | Call Reconnect()          |

## Debug Information

### Connection Debug Output

```
Connecting MQTT server: 192.168.1.100 on port: 1883
Attempting MQTT connection...
MQTT connected!

Published to topic: sensors/temperature
Failed to publish to topic: sensors/humidity

MQTT disconnected
```
