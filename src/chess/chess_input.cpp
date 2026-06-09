#include "chess_input.h"
#include <Arduino.h>

#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 27

namespace Chess {

void input_setup() {
    pinMode(JOY_SW, INPUT_PULLUP);
}

int input_x() {
    return analogRead(JOY_X);
}

int input_y() {
    return analogRead(JOY_Y);
}

bool input_action() {
    static unsigned long lastStateChange = 0;
    static bool buttonState = HIGH;
    bool reading = digitalRead(JOY_SW);
    bool pressed = false;

    if (millis() - lastStateChange > 50) {
        if (reading != buttonState) {
            buttonState = reading;
            lastStateChange = millis();
            if (buttonState == LOW) {
                pressed = true;
            }
        }
    }
    return pressed;
}

bool input_up() {
    static bool lastState = false;
    bool currentState = analogRead(JOY_Y) < 1000;
    bool pressed = (currentState && !lastState);
    lastState = currentState;
    return pressed;
}

bool input_down() {
    static bool lastState = false;
    bool currentState = analogRead(JOY_Y) > 3000;
    bool pressed = (currentState && !lastState);
    lastState = currentState;
    return pressed;
}

bool input_left() {
    static bool lastState = false;
    bool currentState = analogRead(JOY_X) < 1000;
    bool pressed = (currentState && !lastState);
    lastState = currentState;
    return pressed;
}

bool input_right() {
    static bool lastState = false;
    bool currentState = analogRead(JOY_X) > 3000;
    bool pressed = (currentState && !lastState);
    lastState = currentState;
    return pressed;
}

}
