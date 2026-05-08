# SPS30 

  # SPS30 – Particulate Matter Sensor

  ## 1. Component Overview
  **Manufacturer:** Sensirion  
  **Model:** SPS30 (2023/2024)  
  **Revision/Batch:** 2023/2024  
  **Type:** Module  

  **References:**  
  - [Product page](https://sensirion.com/products/catalog/SPS30)  
  - [Datasheet](https://cdn.sparkfun.com/assets/2/d/2/a/6/Sensirion_SPS30_Particulate_Matter_Sensor_v0.9_D1__1_.pdf)  
  - Drivers: [I2C](https://github.com/Sensirion/arduino-i2c-sps30), [UART](https://github.com/Sensirion/arduino-uart-sps30)

  ---

  ## 2. Electrical Characteristics
  | Property            | Value                        |
  |---------------------|-----------------------------|
  | Supply Voltage      | 5.0 V (4.5–5.5 V)           |
  | Idle Current        | <8 mA                       |
  | Peak Current        | 80 mA                       |
  | Logic Level         | 3.3 V                       |
  | Level Shifter Needed| No                          |
  | Power Source        | ESP32 pin, buck-boosted 5.0 V|

  ---

  ## 3. Pinout & Wiring
  ### 3.1 Pin Mapping
  | Pin | Signal | Mainboard Pin | Direction | Notes                      |
  |-----|--------|---------------|-----------|----------------------------|
  | 1   | VDD    | BUCK          | –         |                            |
  | 2   | SDA    | 8             | –         |                            |
  | 3   | SCL    | 9             | –         |                            |
  | 4   | SEL    | GND           | –         | Pull to GND for I²C        |
  | 5   | GND    | GND           | –         |                            |

  ### 3.2 Pull-ups / Pull-downs
  On component: No  
  On mainboard: Yes  
  Values: pulled to ground (down)  
  Signals affected: I2C/UART

  ---

  ## 4. Communication Interface
  +Protocol: I²C  
  +Bus speed: 100 kbit/s  
  +Address: 0x69  
  +Configurable address pins: No  
  +Clock stretching: No  
  +Shared bus: Yes (with SHT41)

  ---

  ## 5. Mode & Control Pins
  | Pin | Function | Default | Required    | Notes         |
  |-----|----------|---------|-------------|---------------|
  | 5   | SELECT   | FLOAT   | GND         | for I²C       |

  +Mode selection: I²C/UART  
  +Reset/Enable: N/A  
  +Power-up: N/A

  ---

  ## 6. Power-Up & Timing
  +Startup delay: <8 s  
  +Warm-up: ~5 s  
  +Measurement interval: –  
  +Blocking: Yes (measures only after warm-up)

  ---

  ## 7. Software Integration
  +Driver/Library: N/A  
  +Target platform: LilyGO T-SIM7080G-S3  
  +ESP32 peripheral: I2C1 (I2C0 used by modem)  
  +Initialization: N/A

  ---

  ## 8. Mechanical & Environmental
  +Mounting: Against airflow  
  +Clearance: N/A  
  +Cable length: UART recommended for >20 cm  
  +Environment: 10–40 °C, 0–95 %RH, airflow required

  ---

  ## 9. Known Issues
  +Failures: No measurement before/after warm-up  
  +Power-cycling: N/A  
  +ESP32-specific: N/A  
  +Workarounds: N/A

  ---

  ## 10. Validation & Testing
  +Test method: Measurement if sensor is detected  
  +Expected behavior: Sensor provides measurements  
  +Voltage/Current: N/A  
  +Criteria: Pass when measurement is received

  ---

  ## 11. References & Diagrams
  +Schematic: N/A  
  +Wiring: N/A  
  +Photos: N/A  
  +Logic Analyzer: N/A

  ---

  ## Change Log
  | Date      | Change      | Reason     |
  |-----------|-------------|------------|
  | 2025-12-25| Population  | init       |
