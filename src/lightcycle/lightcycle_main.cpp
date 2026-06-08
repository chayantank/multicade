#include "lightcycle_main.h"
#include "lightcycle_display.h"
#include "lightcycle_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Lightcycle {

const int SCREEN_W = 128;
const int SCREEN_H = 64;
const int ARENA_Y = 8; // Top HUD area

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;
unsigned long stateTimer = 0;

int score = 0;
int level = 1;

struct Bike {
    int x, y;
    int dir; // 0=Up, 1=Right, 2=Down, 3=Left
    bool alive;
};

Bike p1;
Bike p2;

void initLevel() {
    p1.x = 20; p1.y = 32; p1.dir = 1; p1.alive = true;
    p2.x = 108; p2.y = 32; p2.dir = 3; p2.alive = true;
    
    // Clear screen and draw arena borders once!
    display_clear();
    drawRect(0, ARENA_Y, SCREEN_W, SCREEN_H - ARENA_Y, 1);
    
    // Draw HUD background
    fillRect(0, 0, SCREEN_W, ARENA_Y, 0);
    drawText(2, 0, "LVL:"); drawText(26, 0, level);
    drawText(60, 0, "SCORE:"); drawText(100, 0, score);
    display_render();
}

void resetGame() {
    score = 0;
    level = 1;
    initLevel();
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

unsigned long lastTime = 0;

void loop() {
    unsigned long now = millis();
    
    int inputDir = input_get_direction();

    if (state == STATE_INTRO) {
        if (now - lastTime > 100) {
            lastTime = now;
            display_clear();
            drawText(30, 20, "LIGHTCYCLE");
            drawText(20, 40, "PRESS TO START");
            display_render();
        }
        
        if (inputDir != -1) {
            resetGame();
            state = STATE_PLAYING;
            lastTime = now;
            tone(BUZZER_PIN, 1500, 100);
        }
    } else if (state == STATE_PLAYING) {
        
        // Speed increases with level
        int moveDelay = max(20, 80 - (level * 10)); 
        
        if (now - lastTime > moveDelay) {
            lastTime = now;
            
            // --- Input ---
            if (inputDir != -1) {
                // Prevent 180 turns
                if (inputDir == 0 && p1.dir != 2) p1.dir = 0;
                else if (inputDir == 1 && p1.dir != 3) p1.dir = 1;
                else if (inputDir == 2 && p1.dir != 0) p1.dir = 2;
                else if (inputDir == 3 && p1.dir != 1) p1.dir = 3;
            }
            
            // --- AI Logic (P2) ---
            // Simple look ahead
            int lookX = p2.x;
            int lookY = p2.y;
            if (p2.dir == 0) lookY -= 1;
            else if (p2.dir == 1) lookX += 1;
            else if (p2.dir == 2) lookY += 1;
            else if (p2.dir == 3) lookX -= 1;
            
            // If obstacle ahead, turn
            if (getPixel(lookX, lookY)) {
                // Try turning left or right relative to current direction
                int dirL = (p2.dir + 3) % 4;
                int dirR = (p2.dir + 1) % 4;
                
                int xL = p2.x, yL = p2.y;
                if (dirL == 0) yL--; else if (dirL == 1) xL++; else if (dirL == 2) yL++; else if (dirL == 3) xL--;
                
                int xR = p2.x, yR = p2.y;
                if (dirR == 0) yR--; else if (dirR == 1) xR++; else if (dirR == 2) yR++; else if (dirR == 3) xR--;
                
                bool safeL = !getPixel(xL, yL);
                bool safeR = !getPixel(xR, yR);
                
                if (safeL && safeR) {
                    p2.dir = (random(0, 2) == 0) ? dirL : dirR;
                } else if (safeL) {
                    p2.dir = dirL;
                } else if (safeR) {
                    p2.dir = dirR;
                }
            } else {
                // Occasional random turn just to be tricky
                if (random(0, 100) < 2) { // 2% chance to turn if safe
                    int dirL = (p2.dir + 3) % 4;
                    int dirR = (p2.dir + 1) % 4;
                    int xL = p2.x, yL = p2.y;
                    if (dirL == 0) yL--; else if (dirL == 1) xL++; else if (dirL == 2) yL++; else if (dirL == 3) xL--;
                    int xR = p2.x, yR = p2.y;
                    if (dirR == 0) yR--; else if (dirR == 1) xR++; else if (dirR == 2) yR++; else if (dirR == 3) xR--;
                    
                    if (!getPixel(xL, yL) && random(0, 2) == 0) p2.dir = dirL;
                    else if (!getPixel(xR, yR)) p2.dir = dirR;
                }
            }
            
            // --- Movement & Collision ---
            Bike* bikes[2] = {&p1, &p2};
            bool someoneCrashed = false;
            
            for(int i=0; i<2; i++) {
                if (bikes[i]->alive) {
                    if (bikes[i]->dir == 0) bikes[i]->y -= 1;
                    else if (bikes[i]->dir == 1) bikes[i]->x += 1;
                    else if (bikes[i]->dir == 2) bikes[i]->y += 1;
                    else if (bikes[i]->dir == 3) bikes[i]->x -= 1;
                    
                    // Check collision
                    if (getPixel(bikes[i]->x, bikes[i]->y)) {
                        bikes[i]->alive = false;
                        someoneCrashed = true;
                        
                        // Draw crash explosion
                        drawCircle(bikes[i]->x, bikes[i]->y, 3, 1);
                    } else {
                        // Draw trail
                        drawPixel(bikes[i]->x, bikes[i]->y, 1);
                    }
                }
            }
            
            // Head on collision check
            if (p1.x == p2.x && p1.y == p2.y) {
                p1.alive = false;
                p2.alive = false;
                someoneCrashed = true;
                drawCircle(p1.x, p1.y, 4, 1);
            }
            
            display_render();
            
            if (someoneCrashed) {
                state = STATE_GAMEOVER;
                stateTimer = now;
                tone(BUZZER_PIN, 200, 1000);
            } else {
                if (now % 200 < 50) tone(BUZZER_PIN, 800 + p1.dir*100, 20); // Engine hum
            }
        }
        
    } else if (state == STATE_GAMEOVER) {
        if (now - stateTimer > 1000) { // Wait 1 sec before showing text
            // Don't clear display immediately to show the crash!
            fillRect(20, 20, 88, 24, 0); // Clear a box for text
            drawRect(20, 20, 88, 24, 1);
            
            if (!p1.alive && !p2.alive) {
                drawText(40, 25, "DRAW!");
            } else if (!p1.alive) {
                drawText(40, 25, "YOU LOSE");
            } else {
                drawText(40, 25, "YOU WIN!");
                if (now - stateTimer > 2000 && now - stateTimer < 2100) {
                    score += 100 * level;
                    level++;
                }
            }
            drawText(25, 35, "PRESS TO CONT");
            display_render();
            
            if (inputDir != -1 && now - stateTimer > 2000) {
                if (!p1.alive) {
                    state = STATE_INTRO; // Back to title if lost
                } else {
                    initLevel(); // Next level if won
                    state = STATE_PLAYING;
                }
            }
        }
    }
}

}
