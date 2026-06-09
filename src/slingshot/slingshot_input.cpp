#include "slingshot_input.h"
#include <Arduino.h>

#define VRX_PIN 34
#define VRY_PIN 35
#define SW_PIN 27

namespace Slingshot {

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
    return digitalRead(SW_PIN) == LOW; // returns true when held
}

}
