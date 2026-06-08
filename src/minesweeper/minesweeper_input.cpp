#include "minesweeper_input.h"

#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 27
#define BUZZER_PIN 14

extern int active_game;

namespace Minesweeper {

void input_setup() {
    pinMode(JOY_SW, INPUT_PULLUP);
}

bool input_left() { return analogRead(JOY_X) < 1200; }
bool input_right() { return analogRead(JOY_X) > 2800; }
bool input_up() { return analogRead(JOY_Y) < 1200; }
bool input_down() { return analogRead(JOY_Y) > 2800; }

Action input_action() {
    static uint32_t press_start = 0;
    static bool prev_pressed = false;
    static bool flag_triggered = false;
    
    bool pressed = digitalRead(JOY_SW) == LOW;
    Action result = ACTION_NONE;

    if (pressed) {
        if (!prev_pressed) {
            // Just pressed
            press_start = millis();
            flag_triggered = false;
        } else {
            // Holding
            uint32_t duration = millis() - press_start;
            if (duration >= 300 && duration < 3000 && !flag_triggered) {
                // Trigger flag action immediately at 300ms
                flag_triggered = true;
                tone(BUZZER_PIN, 1200, 50); // High pitched blip for flag
                result = ACTION_FLAG;
            } else if (duration >= 3000) {
                // Exit game
                active_game = 0;
                ESP.restart();
            }
        }
    } else {
        if (prev_pressed) {
            // Just released
            uint32_t duration = millis() - press_start;
            if (duration < 300) {
                // Released before 300ms -> it's a Quick Click
                result = ACTION_REVEAL;
            }
        }
    }
    
    prev_pressed = pressed;
    return result;
}

}
