#include "terraria_input.h"
#include <Arduino.h>

#define VRX_PIN 34
#define VRY_PIN 35
#define SW_PIN 27

extern int active_game;

namespace Terraria {

void input_setup() {
    pinMode(VRX_PIN, INPUT);
    pinMode(VRY_PIN, INPUT);
    pinMode(SW_PIN, INPUT_PULLUP);
}

bool input_left() {
    return analogRead(VRX_PIN) < 1000;
}

bool input_right() {
    return analogRead(VRX_PIN) > 3000;
}

bool input_up() {
    return analogRead(VRY_PIN) < 1000;
}

bool input_down() {
    return analogRead(VRY_PIN) > 3000;
}

bool input_action() {
    static bool lastState = HIGH;
    static unsigned long lastPress = 0;
    bool state = digitalRead(SW_PIN);
    bool pressed = false;
    
    if (state == LOW && lastState == HIGH) {
        if (millis() - lastPress > 200) {
            pressed = true;
            lastPress = millis();
        }
    }
    lastState = state;
    
    static uint32_t hold_start = 0;
    if (state == LOW) {
        if (pressed) hold_start = millis();
        else if (millis() - hold_start >= 3000) {
            active_game = 0;
            ESP.restart();
        }
    }
    
    return pressed;
}

}
