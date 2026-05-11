#pragma once

#include <Arduino.h>
#include <DEBUG.h>

// Debounced push-button reader using internal pull-up.
class PushButton
{
public:
    PushButton(int button_pin);
    // Returns true once per press after debounce interval.
    bool IsPressed();

    bool can_press = true;
    unsigned long press_time = 0;

private:
    const unsigned long debounce_delay_ = 30;
    const int button_pin_;
};
