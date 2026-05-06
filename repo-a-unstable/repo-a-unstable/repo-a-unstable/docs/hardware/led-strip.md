# LED Strip

This project uses an addressable LED strip (WS2812B) to provide visual feedback. The LED strip is
connected to the ESP32 microcontroller, which controls the color and brightness of the LEDs based on
various inputs and conditions.

## Hardware Connections

-   **Data Pin**: Connect the data input of the LED strip to GPIO pin 5 on the ESP32.
-   **Power Supply**: Ensure the LED strip is powered with a suitable 5V power supply.
-   **Ground**: Connect the ground of the LED strip to the ground of the ESP32 to ensure a common
    reference point.

## Software Configuration

The project uses the FastLED library to control the LED strip. Make sure to include the library in
your `platformio.ini` file:

```ini
lib_deps =
    FastLED
```

## Custom Wrapper Class

A custom wrapper class `LEDStrip` is implemented to manage the LED strip operations.

### Functions

The following functions are available in the `LEDStrip` class:

-   `setBrightness(uint8_t brightness)`: Sets the brightness of the LED strip.
-   `clear()`: Turns off all LEDs on the strip.
-   `startLoading()`: Starts a loading animation on the LED strip.
-   `stopLoading()`: Stops the loading animation.
-   `loadingAnimation()`: Updates the loading animation.
-   `fillColor(const CRGB &color)`: Fills the entire strip with a specified color.
-   `toColor(const CRGB &to, unsigned long durationMs = 1000)`: Transitions the strip to a specified
    color over a given duration.
-   `fillGradient(const CRGB &from, const CRGB &to)`: Fills the strip with a gradient between two
    colors.

### Loading Animation

The loading animation is called in the setup loop to provide visual feedback during initialization.
This is done by creating a task on a separate core of the ESP32 to ensure smooth operation without
blocking other processes. The animation continues until the `stopLoading()` function is called.

## Color Look Up Table (LUT)

A color look up table (LUT) is defined to provide a set of predefined colors for easy access. The
LUT should be used with the SPS30 sensor readings to change the LED colors based on air quality
levels.

The LUT is defined as follows:

```cpp
constexpr float maxPM = 250.0f;
static const GradientStop stops[] = {
    {0.0f / maxPM, 0, 255, 0},      // green
    {50.0f / maxPM, 255, 255, 0},   // yellow
    {100.0f / maxPM, 255, 165, 0},  // orange
    {150.0f / maxPM, 255, 0, 0},    // red
    {250.0f / maxPM, 128, 0, 0},  // maroon
};
```

This LUT can be used to map PM2.5 readings to specific colors on the LED strip, providing a visual
indication of air quality.

## Status Indication

This section provides a table which shows how different LED colors correspond to various system
statuses

TODO: Add battery status indications.

| Status        | State     | LED Color  | Description                              |
| ------------- | --------- | ---------- | ---------------------------------------- |
| SPS30 Setup   | Breathing | Deep Sky Blue | The SPS30 sensor is initializing.        |
| SPS30 Error   | Blinking  | Deep Sky Blue | The SPS30 sensor encountered an error.   |
| SHT41 Setup   | Breathing | Dark Blue  | The SHT41 sensor is initializing.        |
| SHT41 Error   | Blinking  | Dark Blue  | The SHT41 sensor encountered an error.   |
| SIM7080 Setup | Breathing | Purple     | The SIM7080 module is initializing.      |
| SIM7080 Error | Blinking  | Purple     | The SIM7080 module encountered an error. |
| SPS30 Reading | Solid     | Green      | Normal air quality.                      |
| SPS30 Reading | Solid     | Yellow     | Moderate air quality.                    |
| SPS30 Reading | Solid     | Orange     | Unhealthy for sensitive groups.          |
| SPS30 Reading | Solid     | Red        | Unhealthy air quality.                   |
| SPS30 Reading | Solid     | Maroon     | Very unhealthy air quality.              |
