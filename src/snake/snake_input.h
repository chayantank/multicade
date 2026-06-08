#pragma once
#include <Arduino.h>

namespace Snake {
    void input_setup();
    bool input_left();
    bool input_right();
    bool input_up();
    bool input_down();
    bool input_fire();
}
