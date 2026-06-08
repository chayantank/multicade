#include "simon_input.h"

#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 27

extern int active_game;

namespace Simon {

void input_setup() {
    pinMode(JOY_SW, INPUT_PULLUP);
}

int input_get_direction() {
    static int last_dir = -1;
    int current_dir = -1;
    
    if (analogRead(JOY_Y) < 1200) current_dir = 0; // UP
    else if (analogRead(JOY_X) > 2800) current_dir = 1; // RIGHT
    else if (analogRead(JOY_Y) > 2800) current_dir = 2; // DOWN
    else if (analogRead(JOY_X) < 1200) current_dir = 3; // LEFT
    
    int result = -1;
    if (current_dir != -1 && last_dir == -1) {
        result = current_dir;
    }
    last_dir = current_dir;
    
    return result;
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
