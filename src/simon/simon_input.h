#pragma once
#include <Arduino.h>

namespace Simon {
    void input_setup();
    int input_get_direction(); // Returns -1 (none), 0 (up), 1 (right), 2 (down), 3 (left)
    bool input_fire();
}
