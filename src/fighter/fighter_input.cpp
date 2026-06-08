#include "fighter_input.h"

#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 27

extern int active_game;

namespace Fighter {

uint32_t btn_hold_start = 0;
bool btn_held = false;

void input_setup() {
    pinMode(JOY_SW, INPUT_PULLUP);
}

// Landscape orientation mapping
// Left -> -Y
// Right -> +Y
// Up -> -X
// Down -> +X

bool input_up() {
    return analogRead(JOY_X) < 1200;
}

bool input_down() {
    return analogRead(JOY_X) > 2800;
}

bool input_left() {
    return analogRead(JOY_Y) < 1200;
}

bool input_right() {
    return analogRead(JOY_Y) > 2800;
}

bool input_button() {
    bool pressed = digitalRead(JOY_SW) == LOW;
    static uint32_t sys_hold_start = 0;
    static bool prev_pressed = false;
    
    bool just_released = false;

    if (pressed) {
        if (!prev_pressed) {
            sys_hold_start = millis();
            btn_hold_start = millis();
            btn_held = true;
        } else if (millis() - sys_hold_start >= 3000) {
            active_game = 0;
            ESP.restart();
        }
    } else {
        if (prev_pressed) {
            just_released = true;
            btn_held = false;
        }
    }
    prev_pressed = pressed;

    // Return true if it was just released, so it acts like a trigger
    // Actually, we want to know if it was just pressed for regular attacks, 
    // or we can use just_released to execute attacks on release (allowing charge checking).
    // Let's use just_released for all attacks.
    return just_released;
}

int input_button_held_time() {
    if (btn_held) {
        return millis() - btn_hold_start;
    }
    return 0; // Or return the last held duration? 0 is fine if we check on release.
}

}
