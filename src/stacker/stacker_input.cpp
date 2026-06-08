#include "stacker_input.h"
#include <Arduino.h>

#define SW_PIN 27

extern int active_game;

namespace Stacker {

void input_setup() {
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

}
