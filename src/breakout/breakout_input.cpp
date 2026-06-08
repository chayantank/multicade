#include "breakout_input.h"

#define JOY_X 34
#define JOY_SW 27

extern int active_game;

namespace Breakout {

void input_setup() {
    pinMode(JOY_SW, INPUT_PULLUP);
}

float input_paddle_speed() {
    int x = analogRead(JOY_X);
    // Center is around 1800-2000. Deadzone from 1600 to 2200.
    if (x > 1600 && x < 2200) return 0.0f;
    
    // Left: 0 to 1600 -> Map to -1.0 to 0.0
    if (x <= 1600) {
        return ((float)x - 1600.0f) / 1600.0f;
    }
    
    // Right: 2200 to 4095 -> Map to 0.0 to 1.0
    return ((float)x - 2200.0f) / (4095.0f - 2200.0f);
}

bool input_fire() {
    bool pressed = digitalRead(JOY_SW) == LOW;
    static uint32_t sw_hold_start = 0;
    static bool prev_pressed = false;

    if (pressed) {
        if (!prev_pressed) {
            sw_hold_start = millis();
        } else if (millis() - sw_hold_start >= 3000) {
            active_game = 0;
            ESP.restart();
        }
    }
    prev_pressed = pressed;

    return pressed;
}

}
