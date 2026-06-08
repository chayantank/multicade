#include <Arduino.h>
#include "input.h"
#include "config.h"

extern int active_game;

namespace Tetris {


// ─── Internal State ───────────────────────────────────
// DAS state for left/right
static bool     prev_raw_left = false;
static bool     prev_raw_right = false;
static uint32_t hold_start_left = 0;
static uint32_t hold_start_right = 0;
static uint32_t last_fire_left = 0;
static uint32_t last_fire_right = 0;

// Edge detection for rotate (joystick up)
static bool prev_raw_up = false;

// Edge detection for click (joystick SW)
static bool     prev_raw_click = false;
static uint32_t last_click_time = 0;

// Per-frame results
static bool res_left = false;
static bool res_right = false;
static bool res_down = false;
static bool res_rotate = false;
static bool res_hard_drop = false;
static bool res_any = false;

// ─── Setup ────────────────────────────────────────────
void input_setup() {
  pinMode(JOY_SW, INPUT_PULLUP);
}

// ─── Update (call once per frame) ─────────────────────
void input_update() {
  uint32_t now = millis();

  // Read raw analog values
  int ax = analogRead(JOY_X);
  int ay = analogRead(JOY_Y);
  bool raw_left  = (ax < JOY_THRESHOLD_LOW);
  bool raw_right = (ax > JOY_THRESHOLD_HIGH);
  bool raw_up    = (ay < JOY_THRESHOLD_LOW);
  bool raw_down  = (ay > JOY_THRESHOLD_HIGH);
  bool raw_click = (digitalRead(JOY_SW) == LOW);

  static uint32_t sw_hold_start = 0;
  if (raw_click) {
      if (!prev_raw_click) {
          sw_hold_start = now;
      } else if (now - sw_hold_start >= 3000) {
          active_game = 0;
          ESP.restart();
      }
  }


  // ── Left with DAS ──
  res_left = false;
  if (raw_left) {
    if (!prev_raw_left) {
      // Just pressed — fire immediately
      res_left = true;
      hold_start_left = now;
      last_fire_left = now;
    } else if (now - hold_start_left > DAS_DELAY_MS &&
               now - last_fire_left > DAS_REPEAT_MS) {
      // Auto-repeat
      res_left = true;
      last_fire_left = now;
    }
  }
  prev_raw_left = raw_left;

  // ── Right with DAS ──
  res_right = false;
  if (raw_right) {
    if (!prev_raw_right) {
      res_right = true;
      hold_start_right = now;
      last_fire_right = now;
    } else if (now - hold_start_right > DAS_DELAY_MS &&
               now - last_fire_right > DAS_REPEAT_MS) {
      res_right = true;
      last_fire_right = now;
    }
  }
  prev_raw_right = raw_right;

  // ── Down (continuous) ──
  res_down = raw_down;

  // ── Rotate (rising edge only) ──
  res_rotate = (raw_up && !prev_raw_up);
  prev_raw_up = raw_up;

  // ── Hard drop (rising edge, debounced) ──
  res_hard_drop = false;
  if (raw_click && !prev_raw_click && (now - last_click_time > DEBOUNCE_MS)) {
    res_hard_drop = true;
    last_click_time = now;
  }
  prev_raw_click = raw_click;

  // ── Any press ──
  res_any = res_left || res_right || res_down || res_rotate || res_hard_drop;
}

// ─── Getters ──────────────────────────────────────────
bool input_left()      { return res_left; }
bool input_right()     { return res_right; }
bool input_down()      { return res_down; }
bool input_rotate()    { return res_rotate; }
bool input_hard_drop() { return res_hard_drop; }
bool input_any_press() { return res_any; }

} // namespace Tetris
