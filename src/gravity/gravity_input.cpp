#include "gravity_input.h"
#include <Arduino.h>

#define SW_PIN 27

namespace Gravity {

void input_setup() {
    pinMode(SW_PIN, INPUT_PULLUP);
}

bool input_action() {
    static bool lastState = HIGH;
    bool state = digitalRead(SW_PIN);
    bool pressed = (state == LOW && lastState == HIGH);
    lastState = state;
    return pressed;
}

}
