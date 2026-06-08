#include "rhythm_main.h"
#include "rhythm_display.h"
#include "rhythm_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Rhythm {

const int HIT_Y = 50;
const int HIT_WINDOW = 8;
const int NOTE_SPEED = 40; // pixels per second

struct Note {
    float y;
    int lane; // 0=L, 1=U, 2=D, 3=R
    int pitch;
    bool active;
};

const int MAX_NOTES = 10;
Note notes[MAX_NOTES];

int score = 0;
int combo = 0;
int maxCombo = 0;
int health = 20;

int laneFlash[4] = {0,0,0,0}; // frames remaining for flash

unsigned long lastSpawn = 0;
int spawnInterval = 1000;

bool gameOver = false;

// Basic melody array
int melody[] = {
    262, 294, 330, 349, 392, 440, 494, 523
};

void spawnNote() {
    for(int i=0; i<MAX_NOTES; i++) {
        if (!notes[i].active) {
            notes[i].active = true;
            notes[i].y = -10;
            notes[i].lane = random(0, 4);
            notes[i].pitch = melody[random(0, 8)];
            return;
        }
    }
}

void resetGame() {
    score = 0;
    combo = 0;
    maxCombo = 0;
    health = 20;
    spawnInterval = 1200;
    for(int i=0; i<MAX_NOTES; i++) notes[i].active = false;
    for(int i=0; i<4; i++) laneFlash[i] = 0;
    gameOver = false;
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
    if (dt > 0.1f) dt = 0.1f;
    
    if (gameOver) {
        display_clear();
        drawText(30, 10, "SONG OVER");
        drawText(30, 30, "SCORE:");
        drawText(70, 30, score);
        drawText(20, 40, "MAX COMBO:");
        drawText(85, 40, maxCombo);
        drawText(15, 55, "PRESS TO RESTART");
        display_render();
        
        if (input_up()) {
            tone(BUZZER_PIN, 800, 100);
            delay(200);
            resetGame();
        }
        return;
    }
    
    if (health <= 0) {
        gameOver = true;
        return;
    }
    
    // Decrease spawn interval over time
    if (now % 5000 < 50 && spawnInterval > 400) {
        spawnInterval--;
    }
    
    if (now - lastSpawn > spawnInterval) {
        spawnNote();
        lastSpawn = now;
    }
    
    // Inputs
    bool btn[4] = {input_left(), input_up(), input_down(), input_right()};
    
    for(int i=0; i<4; i++) {
        if (laneFlash[i] > 0) laneFlash[i]--;
        
        if (btn[i]) {
            laneFlash[i] = 5;
            
            // Check hit
            bool hit = false;
            for(int n=0; n<MAX_NOTES; n++) {
                if (notes[n].active && notes[n].lane == i) {
                    if (abs(notes[n].y - HIT_Y) <= HIT_WINDOW) {
                        // HIT!
                        notes[n].active = false;
                        combo++;
                        if (combo > maxCombo) maxCombo = combo;
                        score += 10 * (1 + combo/10);
                        if (health < 20) health++;
                        tone(BUZZER_PIN, notes[n].pitch, 100);
                        hit = true;
                        break; // only hit one note per press
                    }
                }
            }
            if (!hit) {
                // Miss press
                combo = 0;
                health--;
                tone(BUZZER_PIN, 100, 50); // buzz
            }
        }
    }
    
    // Update notes
    for(int i=0; i<MAX_NOTES; i++) {
        if (notes[i].active) {
            notes[i].y += NOTE_SPEED * dt;
            
            // Check miss
            if (notes[i].y > HIT_Y + HIT_WINDOW) {
                notes[i].active = false;
                combo = 0;
                health -= 2;
                tone(BUZZER_PIN, 50, 100); // low miss sound
            }
        }
    }
    
    // Render
    display_clear();
    
    for(int i=0; i<4; i++) {
        drawLane(i, laneFlash[i]);
    }
    
    for(int i=0; i<MAX_NOTES; i++) {
        if (notes[i].active) {
            drawNote(notes[i].lane, (int)notes[i].y);
        }
    }
    
    // UI
    drawText(2, 2, score);
    if (combo > 5) {
        drawText(2, 12, "x");
        drawText(10, 12, combo);
    }
    
    // HP bar
    drawRect(2, 60, 20, 4, 1);
    if (health > 0) {
        fillRect(2, 60, health, 4, 1);
    }
    
    display_render();
}

}
