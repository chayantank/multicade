#include "gravity_input.h"
#include <Arduino.h>

#define SW_PIN 27

namespace Gravity {

void input_setup() {
    pinMode(SW_PIN, INPUT_PULLUP);
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
