#pragma once

#include <Arduino.h>
#include <vector>

// Color mapping structures and LUT
struct Color
{
    uint8_t r, g, b;
};

struct GradientStop
{
    float offset;
    Color color;
};

class ColorMap
{
public:
    ColorMap(float maxPM, GradientStop stops[]) : _maxPM(maxPM), _stops(stops) {}

    void InitLUT();
    Color DataToColor(float data);

private:
    float _maxPM;
    GradientStop* _stops;
    
    int _stopCount = sizeof(_stops);
    static constexpr int _lutSize = 255;
    Color _lut[_lutSize];
};