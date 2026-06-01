#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include "DEBUG.h"

#define LED_PIN 18
#define NUM_LEDS 14
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

class LEDStrip
{
public:
  enum _loadingModeType
  {
    BREATHING,
    BLINKING
  };

  LEDStrip()
  {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    _initialized = true;
  }

  // Basic controls
  void setBrightness(uint8_t brightness);
  void clear();

  void startLoading(const CRGB &color, _loadingModeType mode = BREATHING);
  void stopLoading();
  bool isLoading() const { return _loadingActive; }

  void loadingAnimation();

  // Fill helpers
  void fillColor(const CRGB &color);
  void toColor(const CRGB &to, unsigned long durationMs = 1000);
  void fillGradient(const CRGB &from, const CRGB &to);

private:
  CRGB _leds[NUM_LEDS];

  int _pin = LED_PIN;
  int _num_leds = NUM_LEDS;
  bool _initialized = false;

  CRGB _current_color = CRGB::Black;

  static void taskEntry(void *arg)
  {
    LEDStrip *self = static_cast<LEDStrip *>(arg);
    self->loadingAnimation();
  }

  TaskHandle_t taskHandle = nullptr;
  volatile bool _loadingActive = false;

  _loadingModeType _loadingMode = BREATHING;
  CRGB _loadingColor = CRGB::Black;
};
