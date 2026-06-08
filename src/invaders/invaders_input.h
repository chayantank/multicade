#ifndef INVADERS_INPUT_H
#define INVADERS_INPUT_H

#include <Arduino.h>

namespace Invaders {
    void input_setup();
    bool input_left();
    bool input_right();
    bool input_fire();
}

#endif
