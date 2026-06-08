#include "motocross_input.h"

#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 27

extern int active_game;

namespace MotoCross {

void input_setup() {
    pinMode(JOY_SW, INPUT_PULLUP);
}

// Landscape orientation mapping
// Up -> -X
// Down -> +X

bool input_up() {
    return analogRead(JOY_X) < 1200;
}

bool input_down() {
    return analogRead(JOY_X) > 2800;
}

bool input_boost() {
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

    return pressed; // Return true as long as held
}

}
