#pragma once
#include <Arduino.h>

namespace PunchOut {
    void input_setup();
    int input_dodge(); // -1 = Left, 1 = Right, 0 = Center
    bool input_punch(); // Returns true on initial press
}
