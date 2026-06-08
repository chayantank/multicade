#pragma once
#include <Arduino.h>

namespace RPG {
    void input_setup();
    float input_x(); // Returns -1.0 to 1.0
    float input_y(); // Returns -1.0 to 1.0
    bool input_attack();
}
