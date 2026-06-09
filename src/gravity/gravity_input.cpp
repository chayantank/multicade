#include "gravity_input.h"
#include <Arduino.h>

#define SW_PIN 27

namespace Gravity {

void input_setup() {
    pinMode(SW_PIN, INPUT_PULLUP);
}

bool input_action() {
    static bool lastState = HIGH;
    static unsigned long lastPress = 0;
    bool state = digitalRead(SW_PIN);
    bool pressed = false;
    if (state == LOW && lastState == HIGH) {
        if (millis() - lastPress > 200) {
            pressed = true;
            lastPress = millis();
        }
    }
    lastState = state;
    return pressed;
}

}
