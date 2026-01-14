# SPS30

## 1.1 SPS30 Software Handling Logic

**Function:** `HandleSPS30Logic()`  
**Purpose:** Periodically reads the SPS30 sensor, updates LED visualization, generates JSON payloads for debug and network posting, and manages a post queue with retry logic.

---

## 1.2 Measurement Timing

```cpp
unsigned long now = millis();
bool can_measure = now - sps30.last_measurement >= kDataMeasurementInterval;
bool can_update = now - sps30.last_update >= kDataTransmissionInterval;
````

* **Explanation:**

  * Checks if enough time has passed since the last measurement (`kDataMeasurementInterval`).
  * Checks if enough time has passed to update LED and JSON (`kDataTransmissionInterval`).
  * If measurement interval not reached, function returns immediately.

---

## 1.3 Reading Sensor Data

```cpp
SPS30_measurement spsData = sps30.readData();

if (!spsData.available)
{
    DEBUG_WARN("Failed to read from sps30 sensor");
    return;
}

sps30.last_measurement = now;
```

* **Explanation:**

  * Reads the SPS30 sensor into an `SPS30_measurement` struct.
  * `spsData.available` indicates if the measurement is valid.
  * During warm-up (~5 s) or communication errors, data is unavailable, and the function exits.
  * Updates `last_measurement` timestamp on success.

---

## 1.4 LED Update

```cpp
SpsDataToColor(spsData.mc_2p0);
```

* **Explanation:**

  * Converts PM2.5 mass concentration to an LED color.
  * Only executed if update interval has elapsed (`can_update`).
  * Provides real-time air quality visualization.

---

## 1.5 JSON Payloads

```cpp
String prettyData = JsonBuilder::BuildJson([&](JsonDocument &doc)
{
    doc["mc_1p0"] = spsData.mc_1p0;
    doc["mc_2p0"] = spsData.mc_2p0;
    doc["mc_4p0"] = spsData.mc_4p0;
    doc["mc_10p0"] = spsData.mc_10p0;
    doc["nc_0p5"] = spsData.nc_0p5;
    doc["nc_1p0"] = spsData.nc_1p0;
    doc["nc_2p5"] = spsData.nc_2p5;
    doc["nc_4p0"] = spsData.nc_4p0;
    doc["nc_10p0"] = spsData.nc_10p0;
    doc["typical_particle_size"] = spsData.typical_particle_size;
    doc["unix_time"] = modem.getCurrentTimestampMs() / 1000;
}, true);
```

* **Explanation:**

  * Builds a **pretty-printed JSON** for debug logging.

```cpp
String data = JsonBuilder::BuildJson([&](JsonDocument &doc)
{
    doc["mc_1p0"] = spsData.mc_1p0;
    doc["mc_2p0"] = spsData.mc_2p0;
    doc["mc_4p0"] = spsData.mc_4p0;
    doc["mc_10p0"] = spsData.mc_10p0;
    doc["nc_0p5"] = spsData.nc_0p5;
    doc["nc_1p0"] = spsData.nc_1p0;
    doc["nc_2p5"] = spsData.nc_2p5;
    doc["nc_4p0"] = spsData.nc_4p0;
    doc["nc_10p0"] = spsData.nc_10p0;
    doc["typical_particle_size"] = spsData.typical_particle_size;
    doc["unix_time"] = modem.getCurrentTimestampMs() / 1000;
});
```

* **Explanation:**

  * Creates a **compact JSON payload** for network transmission.

---

## 1.6 Post Queue Management

```cpp
postQueue.push_back(data);
if (postQueue.size() > kMaxQueueSize)
{
    postQueue.pop_front();
    DEBUG_WARN("Post queue full, dropping oldest data");
}
```

* **Explanation:**

  * Adds payload to a queue for network posting.
  * Ensures queue size does not exceed `kMaxQueueSize`.

```cpp
while (!postQueue.empty())
{
    if (trySendQueue(SPS30path))
    {
        DEBUG_INFO("Successfully sent queued data");
    }
    else
    {
        DEBUG_WARN("Failed to send queued data, will retry later");
        break;
    }
}
```

* **Explanation:**

  * Attempts to send each queued payload.
  * Stops at the first failure; retries on next loop.
  * Ensures reliable data transmission without blocking measurements.

---

## 1.7 Key Notes

* **Warm-up behavior:** SPS30 measurements are invalid for ~5 s after power-up; `spsData.available` ensures no invalid data is used.
* **Non-blocking I²C:** The function returns if intervals are not reached; communication itself does not block.
* **Decoupled intervals:** Measurement and LED/JSON updates are independently timed.
* **Error handling:** Logs warnings for failed reads and full queue but continues operation.

