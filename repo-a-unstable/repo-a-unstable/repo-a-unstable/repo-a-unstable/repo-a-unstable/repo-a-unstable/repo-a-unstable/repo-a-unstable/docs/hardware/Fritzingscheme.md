# Green Mile Project – Hardware Architecture

This document describes the hardware design of the Green Mile project. The system is built around an ESP32 (LILYGO TTGO T-SIM7080G-S3), which controls a WS2812 RGB LED strip and reads data from environmental sensors. The design was created in Fritzing as preparation for developing a custom PCB.

The ESP32 acts as the central controller and communicates with multiple components. It controls the LED strip through a single data connection and reads measurements from an SPS30 particulate matter sensor and an SHT41 temperature and humidity sensor.

The LED strip is connected using a data line and shared power and ground. The data input (DIN) is connected to a GPIO pin on the ESP32, allowing the microcontroller to control the LEDs.

The SPS30 and SHT41 sensors are connected using I2C communication. This means both sensors share the same SDA (data) and SCL (clock) lines connected to the ESP32. They also share the same power and ground connections, which keeps the wiring efficient and suitable for a PCB design.

A breadboard was not used in this project. Instead, all components are directly connected using wired connections. This approach was chosen because the final goal is to design a PCB. By working in this way, the circuit already reflects the final layout more closely, reducing the need for major adjustments later.

The Fritzing design serves as both a visual reference and a starting point for the PCB layout. It can be directly used to place components and route connections in the PCB view.

Overall, this hardware design provides a clear structure in which the ESP32 controls both the LED output and the connected sensors. The decision to avoid a breadboard and work with direct wiring makes the transition to a PCB more efficient and straightforward.

![png](../assets/fritzingdiagramgreenmile_schema.png)
[Fritzing scheme file](../assets/fritzingdiagramgreenmile.fzz)