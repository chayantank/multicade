#pragma once
#include <Arduino.h>

namespace Fighter {
    void input_setup();
    bool input_up();
    bool input_down();
    bool input_left();
    bool input_right();
    bool input_button();
    int input_button_held_time(); // Returns ms the button has been held
}
