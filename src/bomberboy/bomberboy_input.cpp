#include "bomberboy_input.h"
#include <Arduino.h>

#define VRX_PIN 34
#define VRY_PIN 35
#define SW_PIN 27

extern int active_game;

namespace Bomberboy {

void input_setup() {
    pinMode(VRX_PIN, INPUT);
    pinMode(VRY_PIN, INPUT);
    pinMode(SW_PIN, INPUT_PULLUP);
}

int input_get_direction() {
    int x = analogRead(VRX_PIN);
    int y = analogRead(VRY_PIN);
    
    if (y < 1000) return 0; // Up
    if (x > 3000) return 1; // Right
    if (y > 3000) return 2; // Down
    if (x < 1000) return 3; // Left
    
    return -1;
}

bool input_bomb() {
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
