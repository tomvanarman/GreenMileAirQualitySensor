# Segment Display

The segment display is used to show numerical data such as temperature and humidity readings from
the sensors. It is a 8-digit 7-segment display that can display numbers from 00000000 to 99999999.
The display is connected to the ESP32 microcontroller via I2C communication protocol. The display is
controlled using the `LedControl` library, which provides functions to initialize the display, set
the brightness, and display numbers.

## Hardware Connections

-   **Power Supply**: Connect the VCC pin of the segment display to a 5V power supply.
-   **Ground**: Connect the GND pin of the segment display to the ground of the ESP32.
-   **SPI Data In**: Connect the DIN pin of the segment display to GPIO pin 11 on the ESP32.
-   **SPI Clock**: Connect the CLK pin of the segment display to GPIO pin 12 on the ESP32.
-   **SPI Load/Chip Select**: Connect the CS pin of the segment display to GPIO pin 10 on the ESP32.

## Software Configuration

The project uses the `LedControl` library to control the segment display. Make sure to include the
library in your `platformio.ini` file:

```ini
lib_deps =
    LedControl
```

## Custom Wrapper Class

A custom wrapper class `SegmentDisplay` is implemented to manage the segment display operations.

The following functions are available in the `SegmentDisplay` class:

-   `start()`: Initializes the segment display.
-   `setIntensity(int intensity)`: Sets the brightness of the display (0-15).
-   `clearDisplay()`: Clears the entire display.
-   `clearLeft()`: Clears the left 4 digits of the display.
-   `clearRight()`: Clears the right 4 digits of the display.
-   `setTemp(float temperature)`: Displays the temperature on the left 4 digits.
-   `setHum(float humidity)`: Displays the humidity on the right 4 digits.
