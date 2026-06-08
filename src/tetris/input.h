#ifndef _input_h
#define _input_h

#include <stdint.h>

namespace Tetris {


void input_setup();
void input_update();    // Call once per frame

// Movement — returns true with DAS auto-repeat
bool input_left();
bool input_right();

// Soft drop — returns true while held
bool input_down();

// Rotate — returns true on rising edge only (one press = one rotate)
bool input_rotate();

// Hard drop — returns true on rising edge only (debounced click)
bool input_hard_drop();

// Raw button state (for menu navigation)
bool input_any_press();


} // namespace Tetris
#endif
