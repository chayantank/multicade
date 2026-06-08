#ifndef _sound_h
#define _sound_h

namespace Tetris {


void sound_init();
void sound_update();    // Call each frame to handle tone timing

void sound_move();
void sound_rotate();
void sound_hard_drop();
void sound_line_clear();
void sound_level_up();
void sound_game_over();

// Theme Music
void sound_theme_start();
void sound_theme_stop();


} // namespace Tetris
#endif
