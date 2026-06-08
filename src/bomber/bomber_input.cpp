#include "bomber_input.h"

#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 27

extern int active_game;

namespace Bomber {

void input_setup() {
    pinMode(JOY_SW, INPUT_PULLUP);
}

int input_get_direction() {
    if (analogRead(JOY_Y) < 1200) return 0; // UP
    if (analogRead(JOY_X) > 2800) return 1; // RIGHT
    if (analogRead(JOY_Y) > 2800) return 2; // DOWN
    if (analogRead(JOY_X) < 1200) return 3; // LEFT
    return -1;
}

bool input_fire() {
    bool pressed = digitalRead(JOY_SW) == LOW;
    static uint32_t sw_hold_start = 0;
    static bool prev_pressed = false;
    
    bool just_pressed = false;

    if (pressed) {
        if (!prev_pressed) {
            sw_hold_start = millis();
            just_pressed = true;
        } else if (millis() - sw_hold_start >= 3000) {
            active_game = 0;
            ESP.restart();
        }
    }
    prev_pressed = pressed;

    return just_pressed;
}

}
