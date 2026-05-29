#include "PushButton.h"
#include "DEBUG.h"

// Configures the button pin with pull-up for active-low wiring.
PushButton::PushButton(int button_pin) : button_pin_(button_pin)
{
    pinMode(button_pin_, INPUT_PULLUP);
    DEBUG_INFO(String("PushButton initialized on pin ") + String(button_pin_));
}

// Simple debounce logic: returns true on the leading edge if enough time passed.
bool PushButton::IsPressed()
{
    static unsigned long last_press_time = 0;
    const unsigned long now = millis();

    bool pressed = (digitalRead(button_pin_) == LOW);
    if (pressed && (now - last_press_time > debounce_delay_))
    {
        last_press_time = now;
        DEBUG_STEP("BUTTON", "Pressed");
        return true;
    }
    return false;
    // [TODO]: Avoid static state if multiple buttons are used; store last_press_time as a member variable.
}
