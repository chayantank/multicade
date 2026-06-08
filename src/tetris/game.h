#ifndef _game_h
#define _game_h

#include <stdint.h>
#include "config.h"

namespace Tetris {


// ─── Tetromino Data ───────────────────────────────────
// 7 pieces × 4 rotations, each stored as a 4×4 bitmask in a uint16_t.
// Bit layout (row-major, left-to-right):
//   bits [15..12] = row 0
//   bits [11..8]  = row 1
//   bits [7..4]   = row 2
//   bits [3..0]   = row 3
#define NUM_PIECES  7

// Piece type indices
#define PIECE_I  0
#define PIECE_O  1
#define PIECE_T  2
#define PIECE_S  3
#define PIECE_Z  4
#define PIECE_J  5
#define PIECE_L  6

// ─── Game State ───────────────────────────────────────
struct GameState {
  uint8_t  board[BOARD_H][BOARD_W];  // 0 = empty, 1-7 = piece type
  uint8_t  type;                      // current piece (0-6)
  int8_t   px, py;                    // current piece position (top-left of 4×4)
  uint8_t  rotation;                  // 0-3
  uint8_t  nextType;                  // next piece (0-6)
  uint32_t score;
  uint32_t highScore;
  uint8_t  level;
  uint16_t linesCleared;
  bool     gameOver;
};

extern GameState game;

// ─── Piece Helpers ────────────────────────────────────
// Returns true if cell (row, col) is filled in piece at given rotation
bool piece_cell(uint8_t type, uint8_t rot, uint8_t row, uint8_t col);

// ─── Game Functions ───────────────────────────────────
void     game_init();                              // Reset everything, spawn first piece
bool     game_check_collision(int8_t x, int8_t y,
                              uint8_t type, uint8_t rot);
bool     game_move(int8_t dx, int8_t dy);          // Move piece, returns true if moved
bool     game_rotate();                            // Rotate CW with wall kick, returns true if rotated
void     game_lock_piece();                        // Write current piece to board
// Find full lines and return as bitmask (1 << row)
uint32_t game_get_full_lines();
// Remove the lines specified in the mask, shift down, and award points
uint8_t  game_remove_full_lines(uint32_t row_mask);
void     game_hard_drop();                         // Instant drop + lock
void     game_spawn_piece();                       // Spawn next piece, set game over if blocked
int8_t   game_get_ghost_y();                       // Get Y where piece would land
uint16_t game_get_drop_interval();                 // Drop speed for current level

// High score persistence
void     game_load_highscore();
void     game_save_highscore();


} // namespace Tetris
#endif
