# Embedded hardware

## Introduction
This markdown file contains documentation about the embedded hardware components used in the Climate Measuring Box (CMB) and what software toolchain is used for development.

## Toolchain
The embedded software for the CMB is developed using the PlatformIO ecosystem, which is integrated with VSCode. PlatformIO provides a professional development environment for embedded systems, supporting multiple platforms and frameworks. It simplifies the process of managing libraries, building, and uploading firmware to embedded devices.

## Hardware components
The main hardware components used in the CMB project include:

### Microcontrollers
The CMB uses one of the following microcontrollers boards:

- **Lilygo T-SIM7080G** **(Now used)**: This is the main microcontroller board used in the CMB. It has an ESP32 microcontroller and a SIM7080G module for LTE connectivity.
- **WonderBoard** **(Not used)**: This is a custom PCB designed to interface with the various sensors and an LTE module. It also provides a LiFePo4 battery circuit and a RTC. This board is also documented [here](hardware/wonder-board.md). Since we have not gotten the WonderBoard to work properly yet so we are currently using the Lilygo T-SIM7080G as our main board.

### Sensors

- **SPS30**: This is a particulate matter sensor that measures fine particles in the air. [Documentation](hardware/SPS30.md)
- **SHT41**: This is a temperature and humidity sensor. [Documentation](hardware/SHT41.md)

### Wireless communication
The CMB uses the SIM7080G module for LTE connectivity, allowing it to send data to the backend server over the cellular network. This module is integrated into both microcontroller boards used in the project. [Documentation](/docs/software/lte_interface.md)

### Battery
The CMB is powered by a LiFePo4 battery, which offers advantages in terms of safety, temperature tolerance, and lifespan compared to traditional Li-Ion batteries. More information about the battery can be found [here](hardware/LiFePo4.md).

## Frameworks and libraries
The embedded software for the CMB is developed using the Arduino framework, which provides a simple and easy-to-use programming environment for embedded systems. The following libraries are used in the project:

### External libraries
- **TinyGSM**: This library is used to interface with the SIM7080G module for LTE connectivity.
- **Adafruit SHT4x**: This library is used to interface with the SHT40 temperature and humidity sensor.
- **Sensirion SPS30**: This library is used to interface with the SPS30 particulate matter sensor.
- **ArduinoJson**: This library is used for handling JSON data, which is useful for formatting the sensor data before sending it to the backend server.
- **PubSubClient**: This library is used for MQTT communication, allowing the CMB to send data to the backend server.
- **XpowersLib**: This library is used to communicate with the power management IC on the Lilygo T-SIM7080G board to turn off/on peripherals.

### Custom libraries
- **DEBUG**: This library is used for debugging purposes, allowing for easy logging of information during development. The library is made by Sebas.
- **MqttClient**: This is a custom wrapper around the **PubSubClient** library to simplify MQTT communication specific to the CMB project.
- **NetworkServer**: This is a custom library that makes it easy to upload credentials and update configurations over a ESP32 hosted webserver.
- **SHT41Sensor**: This is a custom wrapper around the **Adafruit SHT4x** library to simplify interaction with the SHT41 sensor.
