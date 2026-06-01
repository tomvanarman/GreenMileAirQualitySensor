# How can the ESP32‑S3 and its connected air‑quality sensors be configured to minimize power consumption while maintaining reliable and real‑time measurements in the Green Mile project?

# Introduction

Air‑quality monitoring has become increasingly important as urban environments face rising levels of particulate matter and other pollutants. To address this challenge, the Green Mile project deploys distributed sensor nodes capable of measuring environmental conditions in real time. These nodes rely on the ESP32‑S3 microcontroller, combined with several air‑quality sensors, to collect and transmit data from outdoor locations.

Because the system is battery‑powered and intended for long‑term autonomous operation, power efficiency is a critical design requirement. Each component in the system, from the microcontroller to the particulate matter sensor, contributes to the overall energy budget. The central challenge is therefore to balance two competing goals: minimizing power consumption while still providing reliable and timely air‑quality measurements.

This research investigates how the ESP32‑S3 and its connected sensors can be configured, duty‑cycled, and optimized to reduce energy usage without compromising data quality. By examining the hardware characteristics, power‑saving features, and timing constraints of the system, this study aims to identify an optimal measurement strategy suitable for long‑term outdoor deployment.

## 1 Hardware and Operational Characteristics

To understand how to optimize power consumption in the Green Mile project, it is essential to first examine the specific hardware components used in the prototype. Each component has its own power profile, operational behavior, and constraints that influence the overall energy strategy.

### LILYGO Microcontroller (ESP32‑S3 + SIM + GPS)

The project uses a LILYGO development board that integrates:

- an ESP32‑S3 microcontroller
- a SIM module for mobile data communication
- a GPS receiver for location tracking

This combination makes the board versatile but also increases power demand. Key characteristics include:

- ESP32‑S3 active mode: ~80–240 mA depending on Wi‑Fi/BLE usage (Espressif Systems, 2023a)
- Deep Sleep mode: ~20–30 µA (Espressif Systems, 2023a)
- GPS module: typically 20–30 mA when active
- SIM module: 10–20 mA idle, up to 200–300 mA during transmission bursts

Because the SIM module and GPS are among the most power‑hungry components, careful duty‑cycling is essential.

### 8‑Digit Display (7‑Segment Style)

The device includes an 8‑digit LED display, likely driven by a TM1637/TM1640‑style controller.

Characteristics:

- LED segments require continuous current to stay lit
- Typical consumption: 20–80 mA depending on brightness
- No built‑in low‑power mode

This display is one of the least energy‑efficient components. Power optimization may require:

- lowering brightness
- updating less frequently
- turning it off entirely during sleep cycles

### SHT41 Temperature & Humidity Sensor

The SHT41 is a modern, low‑power digital sensor from Sensirion (Sensirion AG, 2021).  
Key characteristics:

- Measurement current: ~0.4 mA
- Idle current: <1 µA
- Fast measurement time: <10 ms
- Supports low‑power measurement modes

This sensor is highly suitable for battery‑powered applications.

### Sensirion SPS30 Particulate Matter Sensor

The SPS30 is a laser‑based PM sensor (PM1.0, PM2.5, PM4, PM10) (Sensirion AG, 2018).

Characteristics:

- Active measurement: ~60–100 mA
- Fan‑based airflow → cannot measure without powering the fan
- Supports duty‑cycling (e.g., run 10–30 seconds per interval)
- Warm‑up time: ~5–10 seconds (Sensirion AG, 2020)

This sensor is one of the largest contributors to power consumption and must be carefully scheduled.

### WS2812 LED Strip (30 cm)

Based on the hardware inspection, the LED strip used in the project is a WS2812 individually addressable RGB LED strip (Worldsemi, 2019).

Characteristics:

- Operates on 5V
- Uses 3‑wire control: 5V, GND, DATA
- Individually addressable LEDs
- Typical consumption:
  - ~20 mA per LED at full white brightness
  - A 30 cm strip typically contains 9–15 LEDs → 180–300 mA max
- Supports brightness control and selective LED activation

Because WS2812 LEDs scale linearly with brightness, significant power savings can be achieved by:

- reducing brightness
- limiting the number of active LEDs
- disabling the strip during sleep cycles

### Summary of Hardware Impact on Power Consumption

| Component | Typical Power Use | Notes |
| :- | :- | :- |
| ESP32‑S3 | 80–240 mA active, 20–30 µA deep sleep | Strong sleep modes available |
| SIM module | 10–300 mA | High bursts during transmission |
| GPS module | 20–30 mA | Can be duty‑cycled |
| SPS30 | 60–100 mA | Needs fan; duty‑cycling essential |
| SHT41 | <1 mA | Very low power |
| 8‑digit display | 20–80 mA | No sleep mode; brightness matters |
| WS2812 LED strip | 180–300 mA (full white, 30 cm strip) | Brightness + number of LEDs matter |

This hardware overview forms the foundation for determining which power‑saving strategies are feasible and where the biggest gains can be made.

## 2 Existing Power Optimization Methods

### ESP32‑S3 Power‑Saving Features

The ESP32‑S3 provides several mechanisms to reduce energy consumption, which are essential for extending battery life in the Green Mile project (Espressif Systems, 2023b).

**Sleep Modes**

- **Light Sleep**: reduces CPU power while keeping RAM and certain peripherals active.
- **Deep Sleep**: shuts down most components, ideal for long intervals between measurements.
- **ULP Co‑Processor**: can perform simple tasks while the main CPU remains powered down.

**Peripheral Control**

Unused peripherals such as Wi‑Fi, Bluetooth, ADC, SPI, and I²C can be disabled to reduce power draw.

**Dynamic Frequency Scaling**

The CPU clock frequency can be lowered during low‑intensity tasks to reduce active‑mode power consumption.

### Sensor‑Level Power Optimization

The Green Mile project uses two air‑quality sensors: the **SHT41** and the **SPS30**. Both offer opportunities for power optimization.

#### SHT41 (Temperature & Humidity Sensor)

The SHT41 is inherently low‑power and supports:

- Fast measurements (<10 ms)
- Low‑power measurement modes
- Very low idle current (<1 µA)

Because of its efficiency, the SHT41 can be powered only during measurement cycles and turned off immediately afterward.

#### SPS30 (Particulate Matter Sensor)

The SPS30 is more power‑intensive due to its internal fan and laser (Sensirion AG, 2018). However, it supports:

- Duty‑cycling (running the sensor only when needed)
- Short measurement windows (10–30 seconds)
- Stable readings after a warm‑up of 5–10 seconds (Sensirion AG, 2020)

A typical low‑power strategy is:

1. Power the SPS30  
2. Wait for warm‑up  
3. Take measurements  
4. Power it down again  

This significantly reduces average power consumption while maintaining reliable PM data.

## 3 Timing, Delays, and Real‑Time Constraints

Optimizing power consumption introduces timing challenges. Sleep modes and duty‑cycling reduce energy usage but may increase:

- wake‑up latency  
- sensor warm‑up time  
- measurement delays  
- data transmission intervals  

These factors must be balanced to maintain “real‑time” air‑quality monitoring.

### Wake‑Up Latency

- **Light Sleep wake‑up**: ~200 µs  
- **Deep Sleep wake‑up**: ~200–300 ms  

Deep Sleep is far more energy‑efficient but introduces a noticeable delay before the ESP32‑S3 becomes operational again (Espressif Systems, 2023a).

### Sensor Warm‑Up Times (Project‑Specific)

Because the Green Mile project uses the **SHT41** and **SPS30**, their warm‑up behavior directly affects timing.

| Sensor | Warm‑Up Time | Notes |
| :- | :- | :- |
| SHT41 | <1 second | Essentially instant; negligible delay |
| SPS30 | 5–10 seconds | Fan and laser need stabilization (Sensirion AG, 2020) |

The SPS30 is the dominant factor in timing constraints.

### Impact on Real‑Time Data

For the Green Mile project, “real‑time” does not require second‑by‑second updates.  
A measurement interval of **30–60 seconds** is sufficient to capture meaningful air‑quality changes (Sethi & Sarangi, 2017).

This aligns well with:

- deep sleep cycles  
- SPS30 warm‑up requirements  
- battery‑saving strategies  

### Optimal Sampling Strategy

A practical low‑power measurement cycle for this project is:

1. Wake from deep sleep  
2. Power the SHT41 and SPS30  
3. Wait for SPS30 warm‑up (5–10 seconds)  
4. Take measurements  
5. Transmit data via SIM module  
6. Power down sensors  
7. Return to deep sleep  

This approach minimizes active time while maintaining reliable and timely air‑quality data.

# Conclusion

This research shows that the ESP32‑S3 and its connected air‑quality sensors can be configured in a highly energy‑efficient way without compromising the reliability or timeliness of the measurements. The combination of deep sleep modes, peripheral control, and dynamic frequency scaling on the ESP32‑S3 (Espressif Systems, 2023a; 2023b) forms the foundation for significant power savings. By activating the microcontroller only during essential measurement and transmission windows, overall energy consumption can be drastically reduced.

At the sensor level, the SHT41 contributes minimally to the total power budget due to its fast measurement time and extremely low idle current (Sensirion AG, 2021). The SPS30, while more power‑intensive, can still be used efficiently through duty‑cycling and short, controlled measurement intervals after its required warm‑up period (Sensirion AG, 2018; 2020). This ensures accurate particulate matter readings while keeping the average power draw manageable.

The optimal configuration for the Green Mile project is therefore a cyclic measurement strategy in which the ESP32‑S3 wakes from deep sleep, powers the sensors, waits for stabilization, collects data, transmits it, and returns to deep sleep. With measurement intervals of 30–60 seconds, the system maintains real‑time relevance while minimizing active time and energy usage.

In conclusion, by combining microcontroller‑level power management with sensor‑specific duty‑cycling, the Green Mile system can operate efficiently for extended periods on battery power. This makes it well‑suited for long‑term outdoor deployment while still delivering reliable and meaningful air‑quality insights.

# Sources

1. **ESP32‑S3 Documentation**
- Espressif Systems. (2023). *ESP32‑S3 Technical Reference Manual*. Link: https://www.espressif.com/en/support/documents/technical-documents
- Espressif Systems. (2023). *ESP32‑S3 Datasheet*. Link: https://www.espressif.com/en/support/documents/technical-documents

2. **Sensors Used in the Project**
- Sensirion AG. (2021). *SHT4x (SHT40/SHT41/SHT45) Humidity and Temperature Sensor Datasheet*. Link: https://sensirion.com/products/catalog/SHT41
- Sensirion AG. (2018). *SPS30 Particulate Matter Sensor Datasheet*. Link: https://sensirion.com/products/catalog/SPS30

3. **LED Strip**
- Worldsemi. (2019). *WS2812B Intelligent Control LED Datasheet*. Link: https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf

4. **General IoT Power Optimization Literature**
- Sethi, P., & Sarangi, S. R. (2017). Internet of Things: Architectures, Protocols, and Applications. *Journal of Electrical and Computer Engineering*, 2017. https://doi.org/10.1155/2017/9324035
- Zanella, A., Bui, N., Castellani, A., Vangelista, L., & Zorzi, M. (2014). Internet of Things for Smart Cities. *IEEE Internet of Things Journal*, 1(1), 22–32. https://doi.org/10.1109/JIOT.2014.2306328

5. **Additional Relevant References**
- Sensirion AG. (2020). *SPS30 Interface Description*. Link: https://sensirion.com/products/catalog/SPS30
- Espressif Systems. (2023). *ESP-IDF Power Management Documentation*. Link: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/system/power_management.html