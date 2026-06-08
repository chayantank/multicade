#pragma once
#include <Arduino.h>

namespace Breakout {
    void input_setup();
    float input_paddle_speed(); // Returns float between -1.0 and 1.0 based on joystick X
    bool input_fire(); // Handles 3-sec exit
}
