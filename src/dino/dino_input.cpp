#include "dino_input.h"
#include <Arduino.h>

#define VRX_PIN 34
#define VRY_PIN 35
#define SW_PIN 27

extern int active_game;

namespace Dino {

void input_setup() {
    pinMode(VRX_PIN, INPUT);
    pinMode(VRY_PIN, INPUT);
    pinMode(SW_PIN, INPUT_PULLUP);
}

bool input_jump() {
    bool state = digitalRead(SW_PIN);
    bool pressed = (state == LOW);
    
    static uint32_t hold_start = 0;
    static bool lastState = HIGH;
    if (state == LOW) {
        if (lastState == HIGH) hold_start = millis();
        else if (millis() - hold_start >= 3000) {
            active_game = 0;
            ESP.restart();
        }
    }
    lastState = state;

    return (analogRead(VRY_PIN) < 1000) || pressed;
}

bool input_duck() {
    return analogRead(VRY_PIN) > 3000;
}

}
