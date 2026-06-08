#ifndef BOMBERBOY_INPUT_H
#define BOMBERBOY_INPUT_H

namespace Bomberboy {
    void input_setup();
    int input_get_direction(); // 0=Up, 1=Right, 2=Down, 3=Left, -1=None
    bool input_bomb();
}

#endif
