#include <Arduino.h>
#include "HelpMethod.h"

// Instead of busy-waiting, we use a non-blocking wait that allows other tasks to run.
void wait(unsigned long timeout) {
    unsigned long start = millis();
    while (millis() - start < timeout) {
        yield();
    }
}
