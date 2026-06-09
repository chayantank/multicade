#include "gravity_main.h"
#include "gravity_display.h"
#include "gravity_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Gravity {

struct Spikes {
    float x;
    bool isTop;
    bool active;
};

const int MAX_SPIKES = 6;
Spikes spikes[MAX_SPIKES];

float py = 32;
float vy = 0;
float gravity = 300.0f; // Positive = down
int score = 0;
bool gameOver = false;
float speed = 60.0f;

void resetGame() {
    py = 32;
    vy = 0;
    gravity = 300.0f;
    score = 0;
    speed = 60.0f;
    gameOver = false;
    for(int i=0; i<MAX_SPIKES; i++) {
        spikes[i].active = false;
    }
}

void setup() {
    display_setup();
    input_setup();
    resetGame();
}

unsigned long lastTime = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.05f) dt = 0.05f;
    
    if (gameOver) {
        display_clear();
        drawText(35, 20, "GAME OVER");
        drawText(40, 30, score);
        display_render();
        if (input_action()) {
            tone(BUZZER_PIN, 800, 100); delay(200);
            resetGame();
        }
        return;
    }
    
    if (input_action()) {
        gravity = -gravity;
        vy = 0;
        tone(BUZZER_PIN, gravity > 0 ? 400 : 800, 30);
    }
    
    vy += gravity * dt;
    py += vy * dt;
    
    if (py > 58) {
        py = 58;
        vy = 0;
    }
    if (py < 2) {
        py = 2;
        vy = 0;
    }
    
    score++;
    speed += dt * 2.0f;
    
    for(int i=0; i<MAX_SPIKES; i++) {
        if (spikes[i].active) {
            spikes[i].x -= speed * dt;
            if (spikes[i].x < -10) {
                spikes[i].active = false;
            }
            
            float sx = spikes[i].x;
            if (sx < 24 && sx + 10 > 20) {
                if (spikes[i].isTop && py < 17) {
                    gameOver = true;
                    tone(BUZZER_PIN, 100, 300);
                }
                if (!spikes[i].isTop && py > 43) {
                    gameOver = true;
                    tone(BUZZER_PIN, 100, 300);
                }
            }
        }
    }
    
    if (random(0, 40) == 0) {
        for(int i=0; i<MAX_SPIKES; i++) {
            if (!spikes[i].active) {
                bool valid = true;
                for(int j=0; j<MAX_SPIKES; j++) {
                    if (spikes[j].active && spikes[j].x > 90) valid = false;
                }
                if (valid) {
                    spikes[i].active = true;
                    spikes[i].x = 128;
                    spikes[i].isTop = random(0, 2) == 0;
                }
                break;
            }
        }
    }
    
    display_clear();
    
    drawLine(0, 0, 127, 0, 1);
    drawLine(0, 63, 127, 63, 1);
    
    fillRect(20, (int)py, 4, 4, 1);
    
    for(int i=0; i<MAX_SPIKES; i++) {
        if (spikes[i].active) {
            int x = (int)spikes[i].x;
            if (spikes[i].isTop) {
                drawLine(x, 0, x+5, 15, 1);
                drawLine(x+10, 0, x+5, 15, 1);
            } else {
                drawLine(x, 63, x+5, 48, 1);
                drawLine(x+10, 63, x+5, 48, 1);
            }
        }
    }
    
    drawText(2, 2, score / 10);
    
    display_render();
}

}
