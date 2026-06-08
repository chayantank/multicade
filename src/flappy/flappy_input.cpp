#include "flappy_input.h"

#define JOY_SW 27

extern int active_game;

namespace Flappy {

void input_setup() {
    pinMode(JOY_SW, INPUT_PULLUP);
}

bool input_flap() {
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
