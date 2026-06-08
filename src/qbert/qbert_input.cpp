#include "qbert_input.h"
#include <Arduino.h>

#define VRX_PIN 34
#define VRY_PIN 35
#define SW_PIN 27

extern int active_game;

namespace Qbert {

void input_setup() {
    pinMode(VRX_PIN, INPUT);
    pinMode(VRY_PIN, INPUT);
    pinMode(SW_PIN, INPUT_PULLUP);
}

bool input_action() {
    static bool lastState = HIGH;
    bool state = digitalRead(SW_PIN);
    bool pressed = (state == LOW && lastState == HIGH);
    
    static uint32_t hold_start = 0;
    if (state == LOW) {
        if (lastState == HIGH) hold_start = millis();
        else if (millis() - hold_start >= 3000) {
            active_game = 0;
            ESP.restart();
        }
    }
    
    lastState = state;
    return pressed;
}

bool input_upLeft() {
    static bool lastState = false;
    bool state = (analogRead(VRX_PIN) < 1000 && analogRead(VRY_PIN) < 1000);
    bool pressed = (state && !lastState);
    lastState = state;
    return pressed;
}

bool input_upRight() {
    static bool lastState = false;
    bool state = (analogRead(VRX_PIN) > 3000 && analogRead(VRY_PIN) < 1000);
    bool pressed = (state && !lastState);
    lastState = state;
    return pressed;
}

bool input_downLeft() {
    static bool lastState = false;
    bool state = (analogRead(VRX_PIN) < 1000 && analogRead(VRY_PIN) > 3000);
    bool pressed = (state && !lastState);
    lastState = state;
    return pressed;
}

bool input_downRight() {
    static bool lastState = false;
    bool state = (analogRead(VRX_PIN) > 3000 && analogRead(VRY_PIN) > 3000);
    bool pressed = (state && !lastState);
    lastState = state;
    return pressed;
}

}
