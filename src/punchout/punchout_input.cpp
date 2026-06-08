#include "punchout_input.h"

#define JOY_X 34
#define JOY_SW 27

extern int active_game;

namespace PunchOut {

void input_setup() {
    pinMode(JOY_SW, INPUT_PULLUP);
}

int input_dodge() {
    int x = analogRead(JOY_X);
    if (x < 1200) return -1;
    if (x > 2800) return 1;
    return 0;
}

bool input_punch() {
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
