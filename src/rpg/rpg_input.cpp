#include "rpg_input.h"

#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 27

extern int active_game;

namespace RPG {

void input_setup() {
    pinMode(JOY_SW, INPUT_PULLUP);
}

float input_x() {
    int x = analogRead(JOY_X);
    if (x > 1600 && x < 2200) return 0.0f;
    if (x <= 1600) return ((float)x - 1600.0f) / 1600.0f;
    return ((float)x - 2200.0f) / (4095.0f - 2200.0f);
}

float input_y() {
    int y = analogRead(JOY_Y);
    if (y > 1600 && y < 2200) return 0.0f;
    if (y <= 1600) return ((float)y - 1600.0f) / 1600.0f;
    return ((float)y - 2200.0f) / (4095.0f - 2200.0f);
}

bool input_attack() {
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
