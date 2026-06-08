#pragma once
#include <Arduino.h>

namespace Hopper {
    void input_setup();
    int input_get_direction(); // -1=None, 0=Up, 1=Right, 2=Down, 3=Left
    bool input_action(); // Used to reset/start
}
