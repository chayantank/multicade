#include "g2048_input.h"
#include <Arduino.h>

#define VRX_PIN 34
#define VRY_PIN 35
#define SW_PIN 27

extern int active_game;

namespace G2048 {

void input_setup() {
    pinMode(VRX_PIN, INPUT);
    pinMode(VRY_PIN, INPUT);
    pinMode(SW_PIN, INPUT_PULLUP);
}

bool input_up() {
    static bool lastState = false;
    bool state = analogRead(VRY_PIN) < 1000;
    bool pressed = (state && !lastState);
    lastState = state;
    return pressed;
}

bool input_down() {
    static bool lastState = false;
    bool state = analogRead(VRY_PIN) > 3000;
    bool pressed = (state && !lastState);
    lastState = state;
    return pressed;
}

bool input_left() {
    static bool lastState = false;
    bool state = analogRead(VRX_PIN) < 1000;
    bool pressed = (state && !lastState);
    lastState = state;
    return pressed;
}

bool input_right() {
    static bool lastState = false;
    bool state = analogRead(VRX_PIN) > 3000;
    bool pressed = (state && !lastState);
    lastState = state;
    return pressed;
}

bool input_action() {
    static unsigned long lastStateChange = 0;
    static bool buttonState = HIGH;
    bool reading = digitalRead(SW_PIN);
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
    
    static uint32_t hold_start = 0;
    if (reading == LOW) {
        if (pressed) hold_start = millis();
        else if (millis() - hold_start >= 3000) {
            ::active_game = 0;
            ESP.restart();
        }
    }
    
    return pressed;
}

}
