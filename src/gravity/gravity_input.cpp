#include "gravity_input.h"
#include <Arduino.h>

#define SW_PIN 27

namespace Gravity {

void input_setup() {
    pinMode(SW_PIN, INPUT_PULLUP);
}

bool input_action() {
    static unsigned long lastEdgeTime = 0;
    static bool lastReading = HIGH;
    static bool buttonState = HIGH;
    bool pressed = false;

    bool reading = digitalRead(SW_PIN);
    
    if (reading != lastReading) {
        lastEdgeTime = millis();
    }
    
    if (millis() - lastEdgeTime > 50) {
        if (reading != buttonState) {
            buttonState = reading;
            if (buttonState == LOW) {
                pressed = true;
            }
        }
    }
    
    lastReading = reading;
    return pressed;
}

}
