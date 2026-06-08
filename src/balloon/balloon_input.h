#pragma once
#include <Arduino.h>

namespace Balloon {
    void input_setup();
    float input_x(); // -1.0 to 1.0
    bool input_flap();
}
