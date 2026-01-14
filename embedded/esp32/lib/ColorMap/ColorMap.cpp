#include "ColorMap.h"

template <typename T>
constexpr T clamp_val(T v, T lo, T hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

void ColorMap::InitLUT()
{
    for (int i = 0; i < _lutSize; ++i)
    {
        float interpolation = static_cast<float>(i) / static_cast<float>(_lutSize - 1);

        const GradientStop *first = &_stops[0];
        const GradientStop *next = &_stops[_stopCount - 1];

        for (int s = 0; s < _stopCount; ++s)
        {
            if (_stops[s].offset >= interpolation)
            {
                next = &_stops[s];
                break;
            }
            first = &_stops[s];
        }

        float denom = next->offset - first->offset;
        float factor = (denom == 0.0f) ? 0.0f : (interpolation - first->offset) / denom;
        factor = clamp_val(factor, 0.0f, 1.0f);

        _lut[i].r = static_cast<uint8_t>(first->color.r + (next->color.r - first->color.r) * factor + 0.5f);
        _lut[i].g = static_cast<uint8_t>(first->color.g + (next->color.g - first->color.g) * factor + 0.5f);
        _lut[i].b = static_cast<uint8_t>(first->color.b + (next->color.b - first->color.b) * factor + 0.5f);
    }
}

Color ColorMap::DataToColor(float data)
{
    float normalized = clamp_val(data / _maxPM, 0.0f, 1.0f);

    int index = static_cast<int>(normalized * (_lutSize - 1) + 0.5f);
    index = clamp_val(index, 0, _lutSize - 1);

    Color c = _lut[index];
    return c;
}