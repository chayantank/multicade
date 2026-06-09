#include "fishing_input.h"
#include <Arduino.h>

#define SW_PIN 27

namespace Fishing {

void input_setup() {
    pinMode(SW_PIN, INPUT_PULLUP);
}

bool input_action() {
    return digitalRead(SW_PIN) == LOW; // returns true when held
}

}
