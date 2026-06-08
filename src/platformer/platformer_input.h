#pragma once
#include <Arduino.h>

namespace Platformer {
    void input_setup();
    bool input_left();
    bool input_right();
    bool input_jump(); // Maps to Up or Button
    bool input_fire(); // Generic button
}
