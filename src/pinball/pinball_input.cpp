#include "pinball_input.h"
#include <Arduino.h>

#define VRX_PIN 34
#define VRY_PIN 35
#define SW_PIN 27

namespace Pinball {

void input_setup() {
    pinMode(VRX_PIN, INPUT);
    pinMode(VRY_PIN, INPUT);
    pinMode(SW_PIN, INPUT_PULLUP);
}

bool input_left() {
    return analogRead(VRX_PIN) < 1000;
}

bool input_right() {
    return analogRead(VRX_PIN) > 3000;
}

bool input_plunge() {
    return analogRead(VRY_PIN) > 3000;
}

}
