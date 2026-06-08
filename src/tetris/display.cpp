#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/TomThumb.h>
#include "display.h"
#include "config.h"
#include "game.h"

namespace Tetris {


// ─── Display Instance ─────────────────────────────────
// Constructor uses PHYSICAL dimensions; rotation handled by GFX
static Adafruit_SSD1306 oled(DISPLAY_PHYS_W, DISPLAY_PHYS_H, &Wire, -1);

// ─── Setup ────────────────────────────────────────────
void display_setup() {
  if (!oled.begin(SSD1306_SWITCHCAPVCC, I2C_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }
  oled.setRotation(DISPLAY_ROTATION);  // Portrait: 64 wide × 128 tall
  oled.setTextColor(WHITE);
  oled.setTextWrap(false);
  oled.clearDisplay();
  oled.display();
}

void display_clear() {
  oled.clearDisplay();
}

void display_update() {
  oled.display();
}

// ─── Convert Board Coords → Screen Coords ─────────────
// Returns false if the row is in the hidden spawn zone (rows 0–1)
static bool boardToScreen(int8_t col, int8_t row, int16_t &sx, int16_t &sy) {
  if (row < HIDDEN_ROWS) return false;
  sx = BOARD_X + col * BLOCK_W;
  sy = BOARD_Y + (row - HIDDEN_ROWS) * BLOCK_H;
  return true;
}

// ─── Draw a Single Block ──────────────────────────────
static void drawBlock(int8_t col, int8_t row, bool filled) {
  int16_t sx, sy;
  if (!boardToScreen(col, row, sx, sy)) return;

  if (filled) {
    oled.fillRect(sx, sy, BLOCK_W - 1, BLOCK_H - 1, WHITE);
  } else {
    oled.drawRect(sx, sy, BLOCK_W - 1, BLOCK_H - 1, WHITE);
  }
}

// ─── Board Border ─────────────────────────────────────
void display_render_border() {
  // Use all 64 pixels! 
  // Outer border at 0 and 63
  oled.drawRect(0, 0, 64, VISIBLE_ROWS * BLOCK_H + 2, WHITE);
  // Inner border at 1 and 62 (touches the blocks at X=2 and X=61)
  oled.drawRect(1, 0, 62, VISIBLE_ROWS * BLOCK_H + 2, WHITE);
}

// ─── Locked Board Blocks ──────────────────────────────
void display_render_board() {
  for (int r = HIDDEN_ROWS; r < BOARD_H; r++) {
    for (int c = 0; c < BOARD_W; c++) {
      if (game.board[r][c] != 0) {
        drawBlock(c, r, true);
      }
    }
  }
}

// ─── Falling Piece ────────────────────────────────────
void display_render_piece() {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (piece_cell(game.type, game.rotation, r, c)) {
        if (game.py + r >= HIDDEN_ROWS) {
          drawBlock(game.px + c, game.py + r, true);
        }
      }
    }
  }
}

// ─── Ghost Piece ──────────────────────────────────────
void display_render_ghost() {
  int8_t gy = game_get_ghost_y();
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (piece_cell(game.type, game.rotation, r, c)) {
        if (gy + r >= HIDDEN_ROWS) {
          drawBlock(game.px + c, gy + r, false);
        }
      }
    }
  }
}

// ─── HUD Strip (below board) ──────────────────────────
void display_render_hud() {
  // Use TomThumb font for ultra-compact text
  oled.setFont(&TomThumb);
  oled.setTextSize(1);
  
  // Line 1: Score
  oled.setCursor(1, HUD_Y + 5);  // TomThumb baseline is bottom-left
  oled.print(F("SCR:"));
  oled.print(game.score);

  // Level — right side of line 1
  oled.setCursor(40, HUD_Y + 5);
  oled.print(F("LVL:"));
  oled.print(game.level);
  
  oled.setFont(); // Reset to standard font
}

// ─── Next Piece Preview (HUD line 2) ─────────────────
void display_render_next() {
  oled.setFont(&TomThumb);
  oled.setTextSize(1);
  oled.setCursor(1, HUD_Y + 14);
  oled.print(F("NEXT:"));
  oled.setFont(); // Reset to standard font

  // Draw next piece with 3×3 pixel mini-blocks
  int16_t px = 30;
  int16_t py = HUD_Y + 8;

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (piece_cell(game.nextType, 0, r, c)) {
        oled.fillRect(px + c * 3, py + r * 3, 2, 2, WHITE);
      }
    }
  }
}

// ─── Line Clear Animation ───────────────────────────────
void display_animate_line_clear(uint32_t row_mask) {
  // Center-out disintegration (5 steps: col 4&5, then 3&6, 2&7, 1&8, 0&9)
  for (int step = 0; step < 5; step++) {
    int c1 = 4 - step;
    int c2 = 5 + step;

    for (int y = 0; y < BOARD_H; y++) {
      if (row_mask & (1UL << y)) {
        int16_t sx1, sy1, sx2, sy2;
        if (boardToScreen(c1, y, sx1, sy1)) {
          oled.fillRect(sx1, sy1, BLOCK_W, BLOCK_H, BLACK);
        }
        if (boardToScreen(c2, y, sx2, sy2)) {
          oled.fillRect(sx2, sy2, BLOCK_W, BLOCK_H, BLACK);
        }
      }
    }
    oled.display();
    
    // Quick 30ms delay to make the wiping effect visible, while allowing sound to play
    uint32_t start = millis();
    while (millis() - start < 30) {
      delay(1);
    }
  }
}

// ─── Animated Intro Screen ────────────────────────────
void display_intro_frame() {
  oled.clearDisplay();
  oled.setTextColor(WHITE);
  
  uint32_t t = millis();

  // 1. Falling background particles
  for(int i=0; i<6; i++) {
     int x = 4 + (i * 10) % 56;
     int y = ((t / 20) + (i * 30)) % 138 - 10;
     oled.drawRect(x, y, 2, 2, WHITE);
  }

  // 2. Animated sine-wave "TETRIS" title
  oled.setTextSize(2);
  const char* title = "TETRIS";
  for(int i=0; i<6; i++) {
     int y_offset = sin(t / 150.0 + i) * 4;
     oled.setCursor(4 + i * 10, 20 + y_offset);
     oled.print(title[i]);
  }

  // 3. Subtitle
  oled.setTextSize(1);
  oled.setCursor(17, 45);
  oled.print(F("ESP32"));

  // 4. High score
  if (game.highScore > 0) {
    oled.setCursor(10, 65);
    oled.print(F("Best:"));
    oled.setCursor(20, 75);
    oled.print(game.highScore);
  }

  // 5. Blinking "Click Button" (on 600ms, off 200ms)
  if ((t % 800) < 600) {
    oled.setCursor(17, 100);
    oled.print(F("Click"));
    oled.setCursor(14, 110);
    oled.print(F("Button"));
  }

  oled.display();
}

// ─── Animated Game Over Screen ──────────────────────────
void display_game_over_frame() {
  oled.clearDisplay();
  oled.setTextColor(WHITE);

  uint32_t t = millis();

  // Background static noise
  for(int i=0; i<15; i++) {
     int x = random(64);
     int y = random(128);
     oled.drawPixel(x, y, WHITE);
  }

  // Shaking text effect (shakes for the first 1.5 seconds)
  int shake_x = 0;
  if ((t % 2000) < 1500) { // Since we don't have a start time, we'll just have it occasionally glitch!
    shake_x = (t % 100 < 50) ? 1 : -1;
  }

  // Title
  oled.setTextSize(2);
  oled.setCursor(8 + shake_x, 15);
  oled.print(F("GAME"));
  oled.setCursor(8 - shake_x, 35);
  oled.print(F("OVER"));

  // Scores
  oled.setTextSize(1);
  oled.setCursor(2, 65);
  oled.print(F("Score:"));
  oled.print(game.score);

  oled.setCursor(2, 80);
  oled.print(F("Best: "));
  oled.print(game.highScore);

  // Restart prompt blinking
  if ((t % 800) < 600) {
    oled.setCursor(17, 105);
    oled.print(F("Click"));
    oled.setCursor(14, 115);
    oled.print(F("Button"));
  }

  oled.display();
}

} // namespace Tetris
