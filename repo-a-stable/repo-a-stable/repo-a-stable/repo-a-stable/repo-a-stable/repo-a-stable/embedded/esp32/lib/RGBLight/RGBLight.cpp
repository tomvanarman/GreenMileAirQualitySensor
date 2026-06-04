#include "RGBLight.h"

#include "Debug.h"
#include "HelpMethod.h"

RGBLight::RGBLight(int r, int g, int b) : r(r), g(g), b(b) {}

void RGBLight::setup() {
    DEBUG_INFO("Initializing RGBLight");
    // Configure the RGBLight pins as outputs
    pinMode(r, OUTPUT);
    pinMode(g, OUTPUT);
    pinMode(b, OUTPUT);

    setupColorCheck();
    // Set all pins LOW initially (LEDs off)
    digitalWrite(r, LOW);
    digitalWrite(g, LOW);
    digitalWrite(b, LOW);
    DEBUG_INFO("RGBLight setup completed");
}

void RGBLight::setupColorCheck() {
    DEBUG_INFO("Setting up color check");
    digitalWrite(r, HIGH);
    wait(1000);
    digitalWrite(r, LOW);
    digitalWrite(g, HIGH);
    wait(1000);
    digitalWrite(g, LOW);
    digitalWrite(b, HIGH);
    wait(1000);
    digitalWrite(b, LOW);
    DEBUG_INFO("Color check completed");
}

void RGBLight::startInitialization() {
    DEBUG_INFO("Starting RGBLight initialization sequence");
    const int breathSteps = 50;
    const int breathDelay = 20;  // ms
    while (1) {
        // Fade in
        for (int val = 0; val <= 255; val += (255 / breathSteps)) {
            analogWrite(b, val);
            vTaskDelay(breathDelay / portTICK_PERIOD_MS);
        }
        // Fade out
        for (int val = 255; val >= 0; val -= (255 / breathSteps)) {
            analogWrite(b, val);
            vTaskDelay(breathDelay / portTICK_PERIOD_MS);
        }
        // Fade in
        for (int val = 0; val <= 255; val += (255 / breathSteps)) {
            analogWrite(g, val);
            vTaskDelay(breathDelay / portTICK_PERIOD_MS);
        }
        // Fade out
        for (int val = 255; val >= 0; val -= (255 / breathSteps)) {
            analogWrite(g, val);
            vTaskDelay(breathDelay / portTICK_PERIOD_MS);
        }
    }
    analogWrite(r, LOW);
    analogWrite(g, LOW);
    analogWrite(b, LOW);
}

void RGBLight::disable() {
    analogWrite(r, LOW);
    analogWrite(g, LOW);
    analogWrite(b, LOW);
}

void RGBLight::warningBlink() {
    analogWrite(r, 255);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    analogWrite(r, 0);
}

void RGBLight::errorEncountered(SetupError error) {
    DEBUG_WARN("Error encountered, update RGB: " + ToString(error));
    while (1) {
        analogWrite(r, 255);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        analogWrite(r, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    analogWrite(r, 0);
}
