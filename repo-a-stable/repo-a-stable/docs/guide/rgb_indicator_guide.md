## RGB Indicator Guide

### Overview
The climate measuring box uses an addressable RGB LED strip to provide clear, visual feedback about its status and activities. The lights help users quickly understand what the device is doing, even without looking at a screen.

### LED Behaviors & Meanings

#### 1. Solid Color
- The entire strip lights up in a single color.
- Used to indicate a specific status (e.g., ready, error, or a custom state).

#### 2. Loading Animations
The device uses two main loading animations to show ongoing processes:

**a. Breathing**
- The LEDs smoothly fade in and out, creating a “breathing” effect.
- Indicates the device is busy, starting up, or processing something.
- The color of the breathing effect can be set to match the type of activity.

**b. Blinking**
- The LEDs turn on and off in a regular pattern.
- Used for attention or warning signals (e.g., waiting for user action or error state).

#### 3. Color Transitions
- The LEDs can smoothly transition from one color to another.
- This is used to indicate a change in state (e.g., from setup to ready).

#### 4. Gradient
- The strip can display a gradient from one color to another across its length.
- Used for more advanced or decorative feedback.

### Example Color Meanings (Typical Usage)
- **Green:** Device is ready or operating normally.
- **Blue (Breathing):** Device is starting up or connecting.
- **Yellow (Blinking):** Waiting for user input or network.
- **Red (Solid or Blinking):** Error or warning.

> **Note:** The exact color meanings can be customized in the software.


---


## RGB Indicator (Discrete RGB LED)

The following behaviors are implemented in the `RGBLight.cpp` file:

- On setup, each color (red, green, blue) is turned on for 1 second in sequence, then turned off.
- During initialization, the blue LED fades in and out (breathing effect), followed by the green LED fading in and out, in a continuous loop.
- When `errorEncountered` is called, the red LED blinks on and off every 500 ms in an infinite loop.
- When `disable` is called, all LEDs are set to off.

- **Solid green:** All systems go!
- **Breathing blue:** Device is working or connecting.
- **Blinking yellow:** Needs your attention.
- **Solid or blinking red:** Something is wrong—check the device or consult the manual.
