# SPS30 
## Hardware Component Integration

## 1. Component Identification
#### Component name: SPS30
#### Manufacturer: Sensiron
#### Exact model / variant: SPS30 2023/2024
#### Module or bare IC: N/A
#### Revision / batch (if known): 2023/2024
- Datasheet / reference: [Product reference](https://sensirion.com/products/catalog/SPS30),[Product datasheet](https://cdn.sparkfun.com/assets/2/d/2/a/6/Sensirion_SPS30_Particulate_Matter_Sensor_v0.9_D1__1_.pdf)
- Additional references: Drivers: [I2C](https://github.com/Sensirion/arduino-i2c-sps30),
[UART](https://github.com/Sensirion/arduino-uart-sps30)


---

## 2. Electrical Characteristics
#### Supply voltage (nominal): 5.0v
#### Supply voltage (min–max): 4.5-5.5v

#### Current consumption:
  - Idle: <8 mA
  - Peak: 80 mA

#### Logic level:
  - &check; 3.3 V
  - [ ] 5 V
  - [ ] Mixed / tolerant

#### Level shifting required: [ ] Yes / &check; No

#### Power source:
  - [ ] Mainboard regulator
  - [ ] External regulator
  - &check; ESP32 pin, buck-boosted to 5.0v

#### Decoupling / filtering:
  - Capacitors used: N/A
  - Location: N/A

---

## 3. Pinout & Wiring

### 3.1 Pin Mapping
| Component Pin | Signal | Mainboard Pin | Direction | Notes                    |
|---------------|--------|---------------|-----------|--------------------------|
|              1|     VDD|           BUCK|           |                          |
|              2|     SDA|              8|           |                          |
|              3|     SCL|              9|           |                          |
|              4|     SEL|            GND|           | Pull to GND to select I2C|
|              5|     GND|            GND|           |                          |

### 3.2 Pull-ups / Pull-downs
#### Present on component: [ ] Yes / &check; No
#### Present on mainboard: &check; Yes / [ ] No
#### Values: pulled to ground (down)
#### Signals affected: I2C/UART

---

## 4. Communication Interface
#### Protocol:
  - &check; I²C
  - [ ] SPI
  - [ ] UART
  - [ ] Other:

#### Bus speed / baud rate: 100 kbit/s

#### Address / chip select: 0x69

#### Configurable address pins: [ ] Yes / &check; No

#### Clock stretching: [ ] Yes / &check; No / [ ] Unknown

#### Shared bus: &check; Yes / [ ] No
  - Other devices: SHT41

---

## 5. Mode & Control Pins
| Pin | Function | Default State | Required State | Notes  |
|-----|----------|---------------|----------------|--------|
|    5|    SELECT|       FLOATING|   PULLED TO GND| for I²C|

#### Mode selection (e.g. I²C / UART): I²C/UART
#### Reset / enable behavior: N/A
#### Power-up requirements: N/A

---

## 6. Power-Up & Timing Behavior
#### Startup delay before communication: <8s

#### Warm-up time (if applicable): ~5s

#### Measurement / update interval:

#### Blocking behavior:
  - &check; Blocking, will not measure without warm-up
  - [ ] Non-blocking

---

## 7. Software Integration Notes
#### Driver / library used: N/A

#### Target platform: LilyGO T-SIM7080G-S3 

#### ESP32 peripheral used:
  - [ ] I2C0
  - &check; I2C1
  - [ ] UART #

#### GPIO constraints / boot pins avoided: I2C1 is used because I2C0 is already in use by the modem

#### Required initialization sequence: N/A

---

## 8. Mechanical & Physical Considerations
#### Mounting orientation: Against airflow

#### Clearance requirements: N/A

#### Cable length limits: use UART above 20cm of cable length

#### Environmental constraints:
  - Temperature: 10 to 40 °C (optimal)
  - Humidity: 0 to 95 %RH (non-condensing)
  - Airflow: required

---

## 9. Known Issues & Caveats
#### Observed failures: during/before warm up cycle will not give any measurement

#### Power-cycling requirements: N/A

#### ESP32-specific issues: N/A

#### Workarounds implemented: N/A

---

## 10. Validation & Testing
#### Test method: The software will attempt to do a measurement if sensor is detected
#### Expected behavior: The sensor is connected and outputs measurements.
#### Measured voltages: N/A
#### Measured current: N/A
#### Pass / fail criteria: pass, when measurement is received.

---

## 11. Diagrams & References
#### Schematic snippet: N/A
#### Wiring diagram: N/A
#### Photos: N/A
#### Logic analyzer / scope captures: N/A

---

### Change Log
| Date    | Change    | Reason    |
|---------|-----------|-----------|
| 17-12-25| Population|       init|
