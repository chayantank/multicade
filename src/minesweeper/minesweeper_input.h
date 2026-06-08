#pragma once
#include <Arduino.h>

namespace Minesweeper {
    enum Action {
        ACTION_NONE,
        ACTION_REVEAL,
        ACTION_FLAG
    };

    void input_setup();
    bool input_left();
    bool input_right();
    bool input_up();
    bool input_down();
    Action input_action(); // Handles quick click vs long click vs 3-sec exit
}
