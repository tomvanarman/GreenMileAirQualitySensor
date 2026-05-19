#pragma once

#include <Arduino.h>
#include <LedControl.h>

class SegmentDisplay
{
public:
    SegmentDisplay(int dataPin, int clkPin, int csPin);

    void start();
    void setIntensity(int intensity);

    void clearDisplay();
    void clearLeft();
    void clearRight();

    void setTemp(float temperature);
    void setHum(float humidity);
    void setError();
    void setBattery(int percentage);
    void setIPAddress(const String &ipAddress);

private:
    LedControl lc;
};