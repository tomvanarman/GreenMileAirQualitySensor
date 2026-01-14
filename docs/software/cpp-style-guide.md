# C++ Style Guide

This document outlines the C++ coding standards used in this project, based on the
[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

## Overview

We follow the Google C++ Style Guide to ensure consistent, readable, and maintainable code across
the project. This guide covers the specific rules and conventions applied in our ESP32 embedded
project.

## Naming Conventions

### Classes

- Use `PascalCase` for class names
- Example: `WifiManager`, `MqttClient`

**Reference:**
[Google Style Guide - Type Names](https://google.github.io/styleguide/cppguide.html#Type_Names)

```cpp
// Good
class WifiManager {
    // ...
};

class MqttClient {
    // ...
};

// Bad
class wifi_manager
{
    // ...
};
```

### Functions and Methods

- Use `PascalCase` for function and method names
- Example: `Connect()`, `Disconnect()`, `Publish()`

**Reference:**
[Google Style Guide - Function Names](https://google.github.io/styleguide/cppguide.html#Function_Names)

```cpp
// Good
void Connect();
bool IsConnected();
void ReconnectToNetwork();

// Bad
void connect();
bool is_connected();
void reconnect_to_network();
```

### Variables

- Use `snake_case` for local variables and function parameters
- Use `snake_case` with trailing underscore for member variables

**Reference:**
[Google Style Guide - Variable Names](https://google.github.io/styleguide/cppguide.html#Variable_Names)

```cpp
// Good - Local variables
uint64_t start_attempt_time = millis();
uint64_t current_time = millis();

// Good - Member variables
class MqttClient
{
private:
    const char* server_;
    int port_;
    WifiManager& wifi_manager_;
};

// Bad
uint64_t startAttemptTime = millis();  // camelCase
const char* server;                         // Missing trailing underscore
```

### Constants

- Use `k` prefix followed by `PascalCase` for constants
- Use `constexpr` instead of `#define` when possible

**Reference:**
[Google Style Guide - Constant Names](https://google.github.io/styleguide/cppguide.html#Constant_Names)

```cpp
// Good
constexpr char kSsid[] = "test";
constexpr char kPassword[] = "test12345";
constexpr uint64_t kReconnectInterval = 60000;

// Bad
#define SSID "test"
const char* ssid = "test";
const uint64_t RECONNECT_INTERVAL = 60000;
```

## Comments and Documentation

### File-Level Comments

- Use `//` style comments for all documentation
- Avoid `/* */` block comments except for temporarily commenting out code

**Reference:**
[Google Style Guide - Comment Style](https://google.github.io/styleguide/cppguide.html#Comment_Style)

```cpp
// Good
// MQTT client wrapper class for ESP32 devices.
//
// This class provides a simplified interface for MQTT operations on ESP32,
// including connection management, message publishing, and automatic reconnection.

// Bad (avoid block comments for documentation)
/*
 * MQTT client wrapper class for ESP32 devices.
 *
 * This class provides a simplified interface for MQTT operations.
 */
```

### Class Comments

- Provide comprehensive class-level documentation
- Include purpose, usage examples, and dependencies

**Reference:**
[Google Style Guide - Class Comments](https://google.github.io/styleguide/cppguide.html#Class_Comments)

```cpp
// Good
// WiFi connection manager for ESP32 devices.
//
// This class provides a simplified interface for WiFi operations on ESP32,
// including connection management, disconnection, and automatic reconnection.
// Maintains network credentials and handles connection state management.
//
// Example usage:
//   WifiManager wifi("MyNetwork", "password123");
//   wifi.Connect();
//   if (WiFi.isConnected()) {
//     // Network operations
//   }
class WifiManager {
    // ...
};
```

### Function Comments

- Document all public methods with purpose, parameters, and behavior
- Explain preconditions, postconditions, and side effects

**Reference:**
[Google Style Guide - Function Comments](https://google.github.io/styleguide/cppguide.html#Function_Comments)

```cpp
// Good
// Establishes connection to the MQTT broker.
//
// Requires active WiFi connection. If already connected, this is a no-op.
// Logs connection status and error codes on failure.
void Connect();

// Publishes a message to the specified MQTT topic.
//
// Args:
//   topic: MQTT topic string (null-terminated)
//   payload: Message content to publish
//
// Returns silently if not connected to broker.
void Publish(const char* topic, const String& payload);
```

### Implementation Comments

- Explain **why** not **what**
- Document non-obvious implementation details
- Explain business logic and design decisions

```cpp
// Good
void Connect() {
    // Ensure WiFi is connected before attempting MQTT connection
    if (!WiFi.isConnected()) {
        DEBUG_LOG_LN("WiFi not connected. Cannot establish MQTT connection.");
        return;
    }

    // Use a fixed client ID for this ESP32 device
    if (client_.connect("ESP32Client")) {
        DEBUG_LOG_LN("MQTT connected!");
    }
}

// Bad (comments that just repeat the code)
void Connect() {
    // Check if WiFi is connected
    if (!WiFi.isConnected()) {
        // Log message and return
        DEBUG_LOG_LN("WiFi not connected. Cannot establish MQTT connection.");
        return;
    }
}
```

## Code Formatting

For the code formatting, we won't use the Google C++ Style Guide's formatting rules directly.
Instead, we use the default formatting provided by Visual Studio Code.

## Header Files

### Header Guards

- Use `#pragma once` instead of traditional include guards
- Place at the very beginning of header files

**Reference:**
[Google Style Guide - The #define Guard](https://google.github.io/styleguide/cppguide.html#The__define_Guard)

```cpp
// Good
#pragma once

#include <WiFi.h>
#include "WifiManager.h"

class MqttClient {
    // ...
};
```

### Include Order

- System/library headers first
- Project headers second
- Separate groups with blank lines

**Reference:**
[Google Style Guide - Names and Order of Includes](https://google.github.io/styleguide/cppguide.html#Names_and_Order_of_Includes)

```cpp
// Good
#include <PubSubClient.h>
#include <WiFi.h>

#include "WifiManager.h"
#include "DEBUG.h"
```

## Class Design

### Constructor Initialization

- Use member initializer lists
- Initialize members in declaration order
- Prefer initialization over assignment

**Reference:**
[Google Style Guide - Constructor Initializer Lists](https://google.github.io/styleguide/cppguide.html#Constructor_Initializer_Lists)

```cpp
// Good
MqttClient::MqttClient(const char* server, int port, WifiManager& wifi_manager)
    : server_(server),
      port_(port),
      wifi_manager_(wifi_manager),
      esp_client_(),
      client_(esp_client_) {
}

// Bad
MqttClient::MqttClient(const char* server, int port, WifiManager& wifi_manager) {
    server_ = server;
    port_ = port;
    wifi_manager_ = wifi_manager;
}
```

### Access Control

- Declare public members first, then private
- Use clear access level organization

```cpp
// Good
class MqttClient {
public:
    MqttClient(const char* server, int port, WifiManager& wifi_manager);
    void Connect();
    void Disconnect();
    bool IsConnected();

private:
    const char* server_;
    int port_;
    WifiManager& wifi_manager_;
};
```

## Modern C++ Features

### Const Correctness

- Use `const` wherever possible
- Mark methods `const` when they don't modify state

**Reference:**
[Google Style Guide - Use of const](https://google.github.io/styleguide/cppguide.html#Use_of_const)

```cpp
// Good
bool IsConnected() const;
void Publish(const char* topic, const String& payload);

// Parameters passed by const reference
WifiManager(const String& ssid, const String& password);
```

### Prefer constexpr over #define

- Use `constexpr` for compile-time constants
- Provides type safety and scope

```cpp
// Good
constexpr uint64_t kReconnectInterval = 60000;
constexpr int kMqttPort = 1883;

// Bad
#define RECONNECT_INTERVAL 60000
#define MQTT_PORT 1883
```

## Error Handling

### Defensive Programming

- Check preconditions explicitly
- Handle error states gracefully
- Log errors for debugging

```cpp
// Good
void MqttClient::Connect()
{
    // Ensure WiFi is connected before attempting MQTT connection
    if (!WiFi.isConnected())
    {
        DEBUG_LOG_LN("WiFi not connected. Cannot establish MQTT connection.");
        return;
    }

    // Only attempt connection if not already connected
    if (!client_.connected())
    {
        // Connection logic here
    }
}
```

## Documentation Standards

### README and Documentation

- Provide clear usage examples
- Document dependencies and setup requirements
- Include troubleshooting information

### Code Examples in Documentation

- Use realistic, working examples
- Show both correct and incorrect usage
- Include expected output when relevant

```cpp
// Example usage in documentation:
WifiManager wifi("MyNetwork", "password123");
MqttClient mqtt("broker.example.com", 1883, wifi);

void setup()
{
    wifi.Connect();
    if (WiFi.isConnected())
    {
        mqtt.Connect();
    }
}
```

## Project-Specific Conventions

### Debug Logging

- Use consistent debug macros throughout the project
- Include meaningful context in log messages

```cpp
// Good
DEBUG_LOG_LN("Connecting MQTT server: " + String(server_) + " on port: " + String(port_));
DEBUG_LOG_LN("MQTT connection failed, rc=" + String(client_.state()));

// Bad
DEBUG_LOG_LN("Connection failed");  // Not enough context
```

### Global Objects

- Use descriptive names for global instances
- Group related globals together

```cpp
// Good
// Global objects
WifiManager network(kSsid, kPassword);
MqttClient server(kMqttServer, kMqttPort, network);
```

### Timing and Delays

- Use named constants for timing values
- Document why specific delays are needed

```cpp
// Good
constexpr uint64_t kDataTransmissionInterval = 5000;  // 5 seconds between data transmissions
constexpr uint64_t kLoopDelay = 300;  // Small delay to prevent excessive CPU usage

delay(kDataTransmissionInterval);
```

## Tools and Enforcement

### Recommended Tools

- **clang-format**: For automatic code formatting
- **cpplint**: For style checking
- **Static Analysis**: For catching potential issues

### VS Code Integration

- Install C++ extension with IntelliSense
- Configure clang-format for consistent formatting
- Use linting extensions for real-time feedback

## Style Guide Compliance Checklist

When reviewing code, check for:

- [ ] **Naming**: Classes use `PascalCase`, variables use `snake_case`
- [ ] **Constants**: Use `k` prefix with `constexpr`
- [ ] **Comments**: Explain why, not what; use `//` style
- [ ] **Formatting**: 4-space indentation, consistent brace placement
- [ ] **Headers**: `#pragma once`, proper include order
- [ ] **Const**: Use `const` and `constexpr` appropriately
- [ ] **Initialization**: Use member initializer lists
- [ ] **Documentation**: All public methods documented

## Additional Resources

- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) - Complete official
  guide
- [Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/) - Modern C++ best practices
- [cppreference](https://en.cppreference.com/) - C++ language reference
- [Arduino Style Guide](https://www.arduino.cc/en/Reference/StyleGuide) - Arduino-specific
  conventions

## Examples from This Project

### WifiManager Class

```cpp
// Demonstrates proper class structure, naming, and documentation
class WifiManager {
public:
    WifiManager(const String& ssid, const String& password);
    void Connect();
    void Disconnect();
    void Reconnect();

private:
    const String ssid_;     // WiFi network SSID
    const String password_; // WiFi network password
};
```

### MqttClient Class

```cpp
// Shows integration patterns and method documentation
class MqttClient {
public:
    // Constructs an MQTT client with the specified broker configuration.
    //
    // Args:
    //   server: MQTT broker hostname or IP address (must remain valid for object lifetime)
    //   port: MQTT broker port number (typically 1883 for non-SSL)
    //   wifi_manager: Reference to WiFi manager for connectivity checks
    MqttClient(const char* server, int port, WifiManager& wifi_manager);

    void Connect();
    void Publish(const char* topic, const String& payload);
    bool IsConnected();
};
```

This style guide ensures consistency across the project and makes the codebase more maintainable and
readable for all team members.
