#include "hopper_input.h"

#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 27

extern int active_game;

namespace Hopper {

void input_setup() {
    pinMode(JOY_SW, INPUT_PULLUP);
}

int input_get_direction() {
    int x = analogRead(JOY_X);
    int y = analogRead(JOY_Y);
    
    // Portrait orientation mapping
    // Up -> +X
    // Down -> -X
    // Left -> -Y
    // Right -> +Y
    
    if (x > 2800) return 0; // Up
    if (x < 1200) return 2; // Down
    if (y > 2800) return 1; // Right
    if (y < 1200) return 3; // Left
    
    return -1;
}

bool input_action() {
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
