/* ═══════════════════════════════════════════════════════
 *  TETRIS on ESP32
 *  SSD1306 OLED 128×64 + Analog Joystick + Buzzer
 *
 *  Author: Chayan Tank
 * ═══════════════════════════════════════════════════════ */

#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "input.h"
#include "game.h"
#include "sound.h"

namespace Tetris {


// ─── Scene Management ─────────────────────────────────
enum Scene { INTRO, PLAYING, GAME_OVER };
static Scene scene = INTRO;

// ─── Setup ────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);

  display_setup();
  input_setup();
  sound_init();
  game_load_highscore();

  // Seed random from floating analog pin
  randomSeed(analogRead(0) ^ (micros() << 16));

  Serial.println(F("Tetris ready!"));
}

// ─── Intro Screen ─────────────────────────────────────
void loopIntro() {
  sound_theme_start();

  // Wait for any press to start
  while (true) {
    display_intro_frame();
    
    input_update();
    sound_update();
    if (input_hard_drop() || input_rotate()) {
      sound_theme_stop();
      scene = PLAYING;
      return;
    }
    delay(30);
  }
}

// ─── Main Gameplay ────────────────────────────────────
void loopPlaying() {
  game_init();

  uint32_t lastDropTime = millis();
  bool     pieceLanded  = false;
  uint32_t landedTime   = 0;

  while (!game.gameOver) {
    input_update();

    // ── Left / Right ──
    if (input_left()) {
      if (game_move(-1, 0)) {
        sound_move();
        // Reset lock delay if piece was landed and we moved
        if (pieceLanded) { pieceLanded = false; }
      }
    }
    if (input_right()) {
      if (game_move(1, 0)) {
        sound_move();
        if (pieceLanded) { pieceLanded = false; }
      }
    }

    // ── Rotate ──
    if (input_rotate()) {
      if (game_rotate()) {
        sound_rotate();
        if (pieceLanded) { pieceLanded = false; }
      }
    }

    // ── Hard Drop ──
    if (input_hard_drop()) {
      game_hard_drop();
      sound_hard_drop();

      // Lock immediately
      game_lock_piece();
      
      uint32_t full_lines = game_get_full_lines();
      if (full_lines != 0) {
        // Render the board with the locked piece before animating
        display_clear();
        display_render_border();
        display_render_board();
        display_render_hud();
        display_update();
        
        sound_line_clear();
        display_animate_line_clear(full_lines);
        
        uint8_t cleared = game_remove_full_lines(full_lines);
        uint8_t oldLevel = game.level;
        game.linesCleared += cleared;
        game.level = game.linesCleared / LINES_PER_LEVEL + 1;
        if (game.level != oldLevel) {
          sound_level_up();
        }
      }

      game_spawn_piece();

      lastDropTime = millis();
      pieceLanded = false;
      continue;  // Skip to next frame
    }

    // ── Determine Drop Speed ──
    uint16_t dropInterval;
    bool softDropping = input_down();
    if (softDropping) {
      dropInterval = SOFT_DROP_MS;
    } else {
      dropInterval = game_get_drop_interval();
    }

    // ── Auto Drop ──
    if (millis() - lastDropTime >= dropInterval) {
      if (!game_check_collision(game.px, game.py + 1,
                                game.type, game.rotation)) {
        // Piece can move down
        game.py++;
        if (softDropping) {
          game.score++;  // +1 for soft drop
        }
        pieceLanded = false;
      } else {
        // Piece cannot move down
        if (!pieceLanded) {
          // Start lock delay
          pieceLanded = true;
          landedTime = millis();
        } else if (millis() - landedTime >= LOCK_DELAY_MS) {
          // Lock delay expired — lock the piece
          game_lock_piece();
          
          uint32_t full_lines = game_get_full_lines();
          if (full_lines != 0) {
            // Render the board with the locked piece before animating
            display_clear();
            display_render_border();
            display_render_board();
            display_render_hud();
            display_update();
            
            sound_line_clear();
            display_animate_line_clear(full_lines);
            
            uint8_t cleared = game_remove_full_lines(full_lines);
            uint8_t oldLevel = game.level;
            game.linesCleared += cleared;
            game.level = game.linesCleared / LINES_PER_LEVEL + 1;
            if (game.level != oldLevel) {
              sound_level_up();
            }
          }

          game_spawn_piece();
          pieceLanded = false;
        }
      }
      lastDropTime = millis();
    }

    // ── Render ──
    display_clear();
    display_render_border();
    display_render_board();
    display_render_ghost();
    display_render_piece();
    display_render_next();
    display_render_hud();
    display_update();

    // ── Sound ──
    sound_update();
  }

  // ── Game Over ──
  sound_game_over();
  if (game.score > game.highScore) {
    game.highScore = game.score;
    game_save_highscore();
  }
  scene = GAME_OVER;
}

// ─── Game Over Screen ─────────────────────────────────
void loopGameOver() {
  uint32_t start = millis();

  while (true) {
    display_game_over_frame();

    input_update();
    sound_update();
    
    // Prevent accidental skip for the first 1.5 seconds
    if (millis() - start > 1500) {
      if (input_hard_drop() || input_rotate()) {
        scene = INTRO;
        return;
      }
    }
    delay(30);
  }
}

// ─── Main Loop ────────────────────────────────────────
void loop() {
  switch (scene) {
    case INTRO:     loopIntro();    break;
    case PLAYING:   loopPlaying();  break;
    case GAME_OVER: loopGameOver(); break;
  }
}

} // namespace Tetris
