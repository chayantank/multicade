#ifndef _config_h
#define _config_h

namespace Tetris {


// ─── Pin Definitions ──────────────────────────────────
// Joystick (same wiring as DOOM project)
#define JOY_X       34      // Analog X-axis
#define JOY_Y       35      // Analog Y-axis
#define JOY_SW      27      // Click button (INPUT_PULLUP)

// Buzzer
#define BUZZER_PIN  14      // Passive buzzer signal pin

// ─── Display ──────────────────────────────────────────
// Physical SSD1306 is 128×64, rotated to PORTRAIT: 64 wide × 128 tall
#define DISPLAY_PHYS_W  128
#define DISPLAY_PHYS_H  64
#define SCREEN_WIDTH    64      // After rotation
#define SCREEN_HEIGHT   128     // After rotation
#define I2C_ADDR        0x3C
#define DISPLAY_ROTATION 1      // 1 or 3 for portrait (try 3 if upside down)

// Top-aligned: border at y=0, board at y=1
#define BOARD_X     2
#define BOARD_Y     1

// ─── Board Geometry ───────────────────────────────────
#define BLOCK_W      6
#define BLOCK_H      6
#define BOARD_W      10
#define BOARD_H      20
#define HIDDEN_ROWS  2
#define VISIBLE_ROWS 18

// HUD strip below board
// Board bottom: 1 + 108 - 1 = 108, border bottom: 109
// HUD: y=112 to y=127 → 16px (2 lines of text)
#define HUD_Y       112

// ─── Input Timing ─────────────────────────────────────
#define JOY_THRESHOLD_LOW   1200
#define JOY_THRESHOLD_HIGH  2800
#define DAS_DELAY_MS        170
#define DAS_REPEAT_MS       50
#define DEBOUNCE_MS         200

// ─── Game Timing ──────────────────────────────────────
#define BASE_DROP_MS     700
#define DROP_DECREASE_MS 100
#define MIN_DROP_MS      100
#define SOFT_DROP_MS     50
#define LOCK_DELAY_MS    500

// ─── Scoring ──────────────────────────────────────────
#define LINES_PER_LEVEL  10


} // namespace Tetris
#endif
