#ifndef SOKOBAN_INPUT_H
#define SOKOBAN_INPUT_H

namespace Sokoban {
    void input_setup();
    int input_get_direction(); // 0=up, 1=right, 2=down, 3=left, -1=none
    bool input_reset();
}

#endif
