#pragma once
#include <Arduino.h>

#include "ErrorEnum.h"

// clang-format off
class RGBLight {
 private:
  const uint8_t r;
  const uint8_t g;
  const uint8_t b;
  void setupColorCheck();

 public:
  RGBLight(int r, int g, int b);
  void setup();
  void startInitialization();
  void warningBlink();
  void errorEncountered(SetupError error);
  void disable();
};
// clang-format on
