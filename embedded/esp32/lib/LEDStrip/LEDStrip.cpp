#include "LEDStrip.h"

void LEDStrip::setBrightness(uint8_t brightness)
{
  if (!_initialized)
    return;
  FastLED.setBrightness(brightness);

  FastLED.show();
}

void LEDStrip::clear()
{
  if (!_initialized)
    return;
  for (uint16_t i = 0; i < _num_leds; ++i)
  {
    _leds[i] = CRGB::Black;
  }

  _current_color = CRGB::Black;
  FastLED.show();
}

void LEDStrip::startLoading(const CRGB &color, _loadingModeType mode)
{
  if (taskHandle != nullptr)
    return;

  _loadingMode = mode;
  _loadingColor = color;
  _loadingActive = true;

  xTaskCreatePinnedToCore(
      LEDStrip::taskEntry,
      "LEDAnim",
      4096,
      this,
      1,
      &taskHandle,
      1);
}

void LEDStrip::stopLoading()
{
  _loadingActive = false;

  if (taskHandle != nullptr)
  {
    while (eTaskGetState(taskHandle) != eDeleted)
      vTaskDelay(1);

    taskHandle = nullptr;
  }

  // Force reset of RMT
  FastLED.clear(true);
  FastLED.setBrightness(200);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(_leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip);

  FastLED.show();
}

uint16_t calcDelay(int b)
{
    float t = 1.0f - (float)b / 200.0f;  
    t = t * t;                           

    const uint16_t minDelay = 1;
    const uint16_t maxDelay = 25;

    return minDelay + t * (maxDelay - minDelay);
}


void LEDStrip::loadingAnimation()
{
  fillColor(_loadingColor);
  FastLED.show();

  if (_loadingMode == BLINKING)
  {
    for (;;)
    {
      if (!_loadingActive)
        break;

      FastLED.setBrightness(200);
      FastLED.show();
      vTaskDelay(100 / portTICK_PERIOD_MS);

      if (!_loadingActive)
        break;

      FastLED.setBrightness(0);
      FastLED.show();
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
    return;
  }

  else if (_loadingMode == BREATHING)
  {
    for (;;)
    {
      if (!_loadingActive)
        break;

      for (int b = 0; b < 200; b++)
      {
        if (!_loadingActive)
          break;
        FastLED.setBrightness(b);
        FastLED.show();

        vTaskDelay(calcDelay(b) / portTICK_PERIOD_MS);
      }

      for (int b = 200; b >= 0; b--)
      {
        if (!_loadingActive)
          break;
        FastLED.setBrightness(b);
        FastLED.show();
        vTaskDelay(calcDelay(b) / portTICK_PERIOD_MS);
      }
    }
  }

  vTaskDelete(NULL);
}

void LEDStrip::fillColor(const CRGB &color)
{
  if (!_initialized)
    return;
  for (uint16_t i = 0; i < _num_leds; ++i)
  {
    _leds[i] = color;
  }

  _current_color = color;
  FastLED.show();
}

void LEDStrip::toColor(const CRGB &to, unsigned long durationMs)
{
  if (!_initialized)
    return;
  unsigned long startTime = millis();
  unsigned long endTime = startTime + durationMs;

  while (millis() < endTime)
  {
    unsigned long currentTime = millis();
    float t = (float)(currentTime - startTime) / (float)durationMs;
    uint8_t r = _current_color.r + (uint8_t)((to.r - _current_color.r) * t);
    uint8_t g = _current_color.g + (uint8_t)((to.g - _current_color.g) * t);
    uint8_t b = _current_color.b + (uint8_t)((to.b - _current_color.b) * t);
    fillColor(CRGB(r, g, b));
    FastLED.show();
    delay(10); // Small delay to allow for visible transition
  }

  _current_color = to;
  fillColor(to);
  FastLED.show();
}

void LEDStrip::fillGradient(const CRGB &from, const CRGB &to)
{
  if (!_initialized)
    return;
  for (uint16_t i = 0; i < _num_leds; ++i)
  {
    float t = _num_leds > 1 ? (float)i / (float)(_num_leds - 1) : 0.0f;
    uint8_t r = from.r + (uint8_t)((to.r - from.r) * t);
    uint8_t g = from.g + (uint8_t)((to.g - from.g) * t);
    uint8_t b = from.b + (uint8_t)((to.b - from.b) * t);
    _leds[i] = CRGB(r, g, b);
  }

  _current_color = to;
  FastLED.show();
}