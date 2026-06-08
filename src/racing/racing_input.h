#pragma once
#include <Arduino.h>

namespace Racing {
    void input_setup();
    bool input_left();
    bool input_right();
    bool input_fire(); // Handles 3-sec exit
}
