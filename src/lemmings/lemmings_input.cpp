#include "lemmings_input.h"
#include <Arduino.h>

#define VRX_PIN 34
#define VRY_PIN 35
#define SW_PIN 27

namespace Lemmings {

void input_setup() {
    pinMode(VRX_PIN, INPUT);
    pinMode(VRY_PIN, INPUT);
    pinMode(SW_PIN, INPUT_PULLUP);
}

int input_x() {
    return analogRead(VRX_PIN);
}

int input_y() {
    return analogRead(VRY_PIN);
}

bool input_action() {
    static unsigned long lastStateChange = 0;
    static bool buttonState = HIGH;
    bool reading = digitalRead(SW_PIN);
    bool pressed = false;

    if (millis() - lastStateChange > 50) {
        if (reading != buttonState) {
            buttonState = reading;
            lastStateChange = millis();
            if (buttonState == LOW) {
                pressed = true;
            }
        }
    }
    return pressed;
}

}
