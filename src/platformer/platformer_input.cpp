#include "platformer_input.h"

#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 27

extern int active_game;

namespace Platformer {

void input_setup() {
    pinMode(JOY_SW, INPUT_PULLUP);
}

bool input_left() {
    return analogRead(JOY_X) < 1200;
}

bool input_right() {
    return analogRead(JOY_X) > 2800;
}

bool input_jump() {
    return (analogRead(JOY_Y) < 1200) || (digitalRead(JOY_SW) == LOW);
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
