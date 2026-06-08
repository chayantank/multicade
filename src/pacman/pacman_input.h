#ifndef PACMAN_INPUT_H
#define PACMAN_INPUT_H

#include <Arduino.h>

namespace Pacman {
    void input_setup();
    bool input_left();
    bool input_right();
    bool input_up();
    bool input_down();
    bool input_fire();
}

#endif
