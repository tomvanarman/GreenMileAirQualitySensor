#include "SegmentDisplay.h"

SegmentDisplay::SegmentDisplay(int dataPin, int clkPin, int csPin)
    : lc(dataPin, clkPin, csPin, 1) {}

void SegmentDisplay::start()
{
    lc.shutdown(0, false); // Wake up display
    lc.setIntensity(0, 5); // Set brightness level (0 is min, 15 is max)
    lc.clearDisplay(0);    // Clear display
}

void SegmentDisplay::setIntensity(int intensity)
{
    lc.setIntensity(0, intensity);
}

void SegmentDisplay::clearDisplay()
{
    lc.clearDisplay(0);
}

void SegmentDisplay::clearLeft()
{
    for (int i = 4; i < 8; ++i)
    {
        lc.setDigit(0, i, 0, false);
    }
}

void SegmentDisplay::clearRight()
{
    for (int i = 0; i < 4; ++i)
    {
        lc.setDigit(0, i, 0, false);
    }
}

void SegmentDisplay::setTemp(float temperature)
{
    int tempInt = static_cast<int>(temperature * 10); // One decimal place

    lc.setChar(0, 3, ' ', false);
    lc.setDigit(0, 2, (tempInt / 100) % 10, false); // Hundreds
    lc.setDigit(0, 1, (tempInt / 10) % 10, true);   // Tens with decimal point
    lc.setDigit(0, 0, tempInt % 10, false);         // Units
}

void SegmentDisplay::setHum(float humidity)
{
    int humInt = static_cast<int>(humidity * 10); // One decimal place

    lc.setDigit(0, 7, (humInt / 100) % 10, false); // Hundreds
    lc.setDigit(0, 6, (humInt / 10) % 10, true);   // Tens with decimal point
    lc.setDigit(0, 5, humInt % 10, false);         // Units
    lc.setChar(0, 4, ' ', false);
}

void SegmentDisplay::setError()
{
    lc.setDigit(0, 7, 9, false);
    lc.setDigit(0, 6, 9, false);
    lc.setDigit(0, 5, 9, false);
    lc.setDigit(0, 4, 9, false);
    lc.setDigit(0, 3, 9, false);
    lc.setDigit(0, 2, 9, false);
    lc.setDigit(0, 1, 9, false);
    lc.setDigit(0, 0, 9, false);
}

void SegmentDisplay::setBattery(int percentage)
{
    if (percentage < 0)
        percentage = 0;
    if (percentage > 100)
        percentage = 100;

    lc.setDigit(0, 4, (percentage / 100) % 10, false); // Hundreds
    lc.setDigit(0, 3, (percentage / 10) % 10, false);  // Tens
    lc.setDigit(0, 2, percentage % 10, false);         // Units
    lc.setChar(0, 1, '%', false);                      // %
}

void SegmentDisplay::setIPAddress(const String &ipAddress)
{
    clearDisplay();

    int actualIndex = 0;

    for (int i = 0; i < ipAddress.length(); i++)
    {
        char current = ipAddress.charAt(i);
        char next = ipAddress.charAt(i + 1);

        if (current == '.')
        {
            continue; // Handled last itteration
        }
        else if (next == '.')
        {
            lc.setChar(0, 7 - actualIndex, current, true);
        }
        else
        {
            lc.setChar(0, 7 - actualIndex, current, false);
        }

        actualIndex++;
    }
}