#ifndef _display_h
#define _display_h

#include <stdint.h>

namespace Tetris {


void display_setup();
void display_clear();
void display_update();

// Game rendering
void display_render_border();
void display_render_board();
void display_render_piece();
void display_render_ghost();
void display_render_hud();
void display_render_next();
void display_animate_line_clear(uint32_t row_mask);

// Screens
void display_intro_frame();
void display_game_over_frame();


} // namespace Tetris
#endif
