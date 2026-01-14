# JSON Format

Defines the JSON payloads exchanged between the embedded device and backend.

## Backend Data Model

Example interface (TypeScript) for persisted SHT41 sensor rows:

```typescript
export interface SHT41Row {
    id: number; // DB-assigned primary key (not sent by device)
    temperature: number; // Celsius
    humidity: number; // %RH
    time_unix: number; // UTC seconds since epoch
}
```

On ingestion the backend ignores any client-sent id field (device should not send one).

## Embedded System Payloads

Payloads are built with ArduinoJson (dynamic document). Wrapper:

```cpp
#pragma once
#include <ArduinoJson.h>
class JsonBuilder {
public:
    static String BuildJson(std::function<void(JsonDocument&)> builder, bool pretty = false) {
        JsonDocument doc;      // Keep small to minimize RAM (current payload is tiny)
        builder(doc);
        String out;
        if (pretty) serializeJsonPretty(doc, out);
        else        serializeJson(doc, out);
        return out;
    }
};
```

Current single‑reading payload (compact form used for transmission):

```json
{
    "temperature": 22.31,
    "humidity": 55.72,
    "time_unix": 1732453142
}
```

Fields:

-   temperature: float (Celsius)
-   humidity: float (% relative humidity)
-   time_unix: integer UTC seconds (modem time via getCurrentTimestampMs()/1000)

Source of time: LTE network time (AT+CCLK?) after CLTS enable. If parsing fails, consider omitting
time_unix or sending 0 and handling server-side.

## Example Build (from main loop)

```cpp
String data = JsonBuilder::BuildJson([&](JsonDocument &doc) {
    doc["temperature"] = shtData.temperature;
    doc["humidity"]    = shtData.humidity;
    doc["time_unix"]   = modem.getCurrentTimestampMs() / 1000;
});
modem.httpPost(host, path, url, data.c_str());
```

Pretty form is only emitted to serial logs:

```cpp
String debugPretty = JsonBuilder::BuildJson([&](JsonDocument &doc) {
    doc["temperature"] = shtData.temperature;
    doc["humidity"]    = shtData.humidity;
    doc["time_unix"]   = modem.getCurrentTimestampMs() / 1000;
}, true);
```