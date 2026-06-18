<a id="rgb-indicator-guide"></a>
## 1. RGB Indicator Guide

<a id="why-this-document-exists"></a>
### Why this document exists
This guide documents the exact behavior implemented for the device's discrete 3‑pin RGB indicator (`RGBLight`). It exists so maintainers and integrators know the precise, code-backed behavior used for wiring checks, initialization, warnings, errors and shutdown. The document deliberately lists only behaviors that are implemented in source.

### Table of contents
- [1. RGB Indicator Guide](#rgb-indicator-guide)
  - [1.1 Why this document exists](#why-this-document-exists)
  - [1.2 Discrete RGB Indicator (`RGBLight`)](#discrete-rgb-indicator-rgblight)
    - [1.2.1 `setup()`](#setup)
    - [1.2.2 `setupColorCheck()`](#setupcolorcheck)
    - [1.2.3 `startInitialization()`](#startinitialization)
    - [1.2.4 `warningBlink()`](#warningblink)
    - [1.2.5 `errorEncountered()`](#errorencountered)
    - [1.2.6 `disable()`](#disable)

<a id="discrete-rgb-indicator-rgblight"></a>
### 1.2 Discrete RGB Indicator (`RGBLight`)

<a id="setup"></a>
#### 1.2.1 `setup()`
- Configures the three GPIO pins (red, green, blue) as outputs and sets them LOW (all off).

<a id="setupcolorcheck"></a>
#### 1.2.2 `setupColorCheck()`
- Turns each color pin HIGH for 1000 ms in sequence (red → green → blue) to allow a wiring/LED check, then sets them LOW.

<a id="startinitialization"></a>
#### 1.2.3 `startInitialization()`
- Runs a continuous initialization sequence that breathes the blue LED, then breathes the green LED, repeatedly.
- Breathing parameters: `breathSteps = 50`, `breathDelay = 20 ms` (PWM steps increment/decrement across ~50 steps).
- Note: this function contains an infinite loop designed to run as a task until externally stopped.

<a id="warningblink"></a>
#### 1.2.4 `warningBlink()`
- Performs a single short red blink: red set to full PWM (255) for ~100 ms, then off.

<a id="errorencountered"></a>
#### 1.2.5 `errorEncountered()`
- Enters an infinite loop blinking red on for 500 ms and off for 500 ms. Intended for fatal/setup errors; it does not return.

<a id="disable"></a>
#### 1.2.6 `disable()`
- Sets all three color pins to LOW (turns LEDs off) via `analogWrite(..., LOW)`.

> Only the discrete `RGBLight` behavior is documented here; references to addressable LED strips have been removed because this unit uses the 3‑pin RGB indicator.
