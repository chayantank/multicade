#pragma once
#include <Arduino.h>

namespace Barrel {
    void input_setup();
    bool input_left();
    bool input_right();
    bool input_up();
    bool input_down();
    bool input_jump(); // True if just pressed
}
