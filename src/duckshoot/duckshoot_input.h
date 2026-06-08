#pragma once
#include <Arduino.h>

namespace DuckShoot {
    void input_setup();
    float input_x(); // -1.0 to 1.0
    float input_y(); // -1.0 to 1.0
    bool input_fire(); // true if just pressed
}
