#include "duckshoot_input.h"

#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 27

extern int active_game;

namespace DuckShoot {

void input_setup() {
    pinMode(JOY_SW, INPUT_PULLUP);
}

float input_x() {
    int val = analogRead(JOY_X);
    // 0 to 4095, center ~2048. Right is higher, Left is lower.
    float normalized = (val - 2048) / 2048.0f;
    if (abs(normalized) < 0.15f) return 0.0f; // Deadzone
    return normalized;
}

float input_y() {
    int val = analogRead(JOY_Y);
    // Up is higher, Down is lower.
    float normalized = (val - 2048) / 2048.0f;
    if (abs(normalized) < 0.15f) return 0.0f;
    return -normalized; // Invert so Up is negative
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
