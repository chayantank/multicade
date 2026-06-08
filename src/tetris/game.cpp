#include <Arduino.h>
#include <Preferences.h>
#include <pgmspace.h>
#include "game.h"
#include "config.h"

namespace Tetris {


// ─── Global Game State ────────────────────────────────
GameState game;

// ─── Tetromino Shapes (SRS Standard) ──────────────────
// Each uint16_t encodes a 4×4 grid. Bit 15 = (row0,col0), bit 0 = (row3,col3).
//
// Pieces: I=0, O=1, T=2, S=3, Z=4, J=5, L=6
// Each piece has 4 rotations: [0]=spawn, [1]=CW, [2]=180, [3]=CCW

const uint16_t TETROMINOES[NUM_PIECES][4] PROGMEM = {
  // I
  { 0x0F00, 0x2222, 0x00F0, 0x4444 },
  // O
  { 0x6600, 0x6600, 0x6600, 0x6600 },
  // T
  { 0x4E00, 0x4640, 0x0E40, 0x4C40 },
  // S
  { 0x6C00, 0x4620, 0x06C0, 0x8C40 },
  // Z
  { 0xC600, 0x2640, 0x0C60, 0x4C80 },
  // J
  { 0x8E00, 0x6440, 0x0E20, 0x44C0 },
  // L
  { 0x2E00, 0x4460, 0x0E80, 0xC440 },
};

// Line clear scoring: 0, 1, 2, 3, 4 lines
const uint16_t LINE_SCORES[] = { 0, 100, 300, 500, 800 };

// ─── Preferences ──────────────────────────────────────
static Preferences prefs;

// ─── Piece Cell Helper ────────────────────────────────
bool piece_cell(uint8_t type, uint8_t rot, uint8_t row, uint8_t col) {
  if (type >= NUM_PIECES || rot >= 4 || row >= 4 || col >= 4) return false;
  uint16_t shape = pgm_read_word(&TETROMINOES[type][rot]);
  return (shape >> (15 - (row * 4 + col))) & 1;
}

// ─── Collision Detection ──────────────────────────────
// Returns true if the piece at (x, y) with given type/rot collides
bool game_check_collision(int8_t x, int8_t y, uint8_t type, uint8_t rot) {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (piece_cell(type, rot, r, c)) {
        int bx = x + c;
        int by = y + r;
        // Wall and floor checks
        if (bx < 0 || bx >= BOARD_W || by >= BOARD_H) return true;
        // Board collision (skip cells above the board — allows spawning)
        if (by >= 0 && game.board[by][bx] != 0) return true;
      }
    }
  }
  return false;
}

// ─── Movement ─────────────────────────────────────────
bool game_move(int8_t dx, int8_t dy) {
  if (!game_check_collision(game.px + dx, game.py + dy, game.type, game.rotation)) {
    game.px += dx;
    game.py += dy;
    return true;
  }
  return false;
}

// ─── Rotation with Wall Kick ──────────────────────────
bool game_rotate() {
  uint8_t newRot = (game.rotation + 1) % 4;

  // Try normal position
  if (!game_check_collision(game.px, game.py, game.type, newRot)) {
    game.rotation = newRot;
    return true;
  }
  // Try shift left by 1
  if (!game_check_collision(game.px - 1, game.py, game.type, newRot)) {
    game.px--;
    game.rotation = newRot;
    return true;
  }
  // Try shift right by 1
  if (!game_check_collision(game.px + 1, game.py, game.type, newRot)) {
    game.px++;
    game.rotation = newRot;
    return true;
  }
  // I-piece may need shift by 2
  if (game.type == PIECE_I) {
    if (!game_check_collision(game.px - 2, game.py, game.type, newRot)) {
      game.px -= 2;
      game.rotation = newRot;
      return true;
    }
    if (!game_check_collision(game.px + 2, game.py, game.type, newRot)) {
      game.px += 2;
      game.rotation = newRot;
      return true;
    }
  }

  return false;  // Rotation failed
}

// ─── Lock Piece to Board ──────────────────────────────
void game_lock_piece() {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (piece_cell(game.type, game.rotation, r, c)) {
        int bx = game.px + c;
        int by = game.py + r;
        if (by >= 0 && by < BOARD_H && bx >= 0 && bx < BOARD_W) {
          game.board[by][bx] = game.type + 1;  // 1-7 (0 = empty)
        }
      }
    }
  }
}

// ─── Line Clearing ────────────────────────────────────
uint32_t game_get_full_lines() {
  uint32_t mask = 0;
  for (int y = 0; y < BOARD_H; y++) {
    bool full = true;
    for (int x = 0; x < BOARD_W; x++) {
      if (game.board[y][x] == 0) {
        full = false;
        break;
      }
    }
    if (full) {
      mask |= (1UL << y);
    }
  }
  return mask;
}

uint8_t game_remove_full_lines(uint32_t row_mask) {
  uint8_t cleared = 0;

  for (int y = BOARD_H - 1; y >= 0; y--) {
    if (row_mask & (1UL << y)) {
      cleared++;
      // Shift everything above this row down by one
      for (int yy = y; yy > 0; yy--) {
        for (int x = 0; x < BOARD_W; x++) {
          game.board[yy][x] = game.board[yy - 1][x];
        }
      }
      // Clear the top row
      for (int x = 0; x < BOARD_W; x++) {
        game.board[0][x] = 0;
      }
      // Also shift the mask down!
      row_mask = (row_mask & ~((1UL << (y + 1)) - 1)) | ((row_mask & ((1UL << y) - 1)) << 1);
      y++;  // Re-check this row (now has content from above)
    }
  }

  // Award score
  if (cleared > 0) {
    game.score += LINE_SCORES[cleared];
    game.linesCleared += cleared;
    
    // Level up every 10 lines
    int new_level = (game.linesCleared / LINES_PER_LEVEL) + 1;
    if (new_level > game.level) {
      game.level = new_level;
      // You could trigger a level up sound/animation here if desired!
    }
  }

  return cleared;
}

// ─── Hard Drop ────────────────────────────────────────
void game_hard_drop() {
  int rows = 0;
  while (!game_check_collision(game.px, game.py + 1, game.type, game.rotation)) {
    game.py++;
    rows++;
  }
  game.score += rows * 2;  // +2 points per row dropped
}

// ─── Ghost Piece Y ────────────────────────────────────
int8_t game_get_ghost_y() {
  int8_t gy = game.py;
  while (!game_check_collision(game.px, gy + 1, game.type, game.rotation)) {
    gy++;
  }
  return gy;
}

// ─── Drop Interval ────────────────────────────────────
uint16_t game_get_drop_interval() {
  int ms = BASE_DROP_MS - ((game.level - 1) * DROP_DECREASE_MS);
  if (ms < MIN_DROP_MS) ms = MIN_DROP_MS;
  return (uint16_t)ms;
}

// ─── Spawn Piece ──────────────────────────────────────
void game_spawn_piece() {
  game.type = game.nextType;
  game.nextType = random(NUM_PIECES);
  game.px = 3;
  game.py = 0;
  game.rotation = 0;

  // If the new piece immediately collides, game over
  if (game_check_collision(game.px, game.py, game.type, game.rotation)) {
    game.gameOver = true;
  }
}

// ─── Initialize Game ──────────────────────────────────
void game_init() {
  // Clear board
  memset(game.board, 0, sizeof(game.board));

  // Reset state
  game.score = 0;
  game.level = 1;
  game.linesCleared = 0;
  game.gameOver = false;

  // Generate first two pieces
  game.nextType = random(NUM_PIECES);
  game_spawn_piece();
}

// ─── High Score Persistence ───────────────────────────
void game_load_highscore() {
  prefs.begin("tetris", true);  // read-only
  game.highScore = prefs.getUInt("highscore", 0);
  prefs.end();
}

void game_save_highscore() {
  prefs.begin("tetris", false);  // read-write
  prefs.putUInt("highscore", game.highScore);
  prefs.end();
}

} // namespace Tetris
