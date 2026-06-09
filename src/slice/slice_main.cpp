#include "slice_main.h"
#include "slice_display.h"
#include "slice_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Slice {

struct Fruit {
    float x, y;
    float vx, vy;
    bool active;
    bool bomb;
};

const int MAX_FRUITS = 5;
Fruit fruits[MAX_FRUITS];

float cursorX = 64;
float cursorY = 32;

float trailX[5];
float trailY[5];
int trailIdx = 0;
bool isSlicing = false;

int score = 0;
int lives = 3;
bool gameOver = false;

unsigned long lastSpawn = 0;
float spawnRate = 2000;

void resetGame() {
    score = 0;
    lives = 3;
    gameOver = false;
    spawnRate = 2000;
    for(int i=0; i<MAX_FRUITS; i++) fruits[i].active = false;
    for(int i=0; i<5; i++) { trailX[i] = cursorX; trailY[i] = cursorY; }
    cursorX = 64;
    cursorY = 32;
}

void setup() {
    display_setup();
    input_setup();
    resetGame();
}

bool lineIntersectsCircle(float x1, float y1, float x2, float y2, float cx, float cy, float r) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float fx = x1 - cx;
    float fy = y1 - cy;
    
    float a = dx*dx + dy*dy;
    float b = 2*(fx*dx + fy*dy);
    float c = (fx*fx + fy*fy) - r*r;
    
    if (a == 0) return false;
    
    float discriminant = b*b - 4*a*c;
    if(discriminant >= 0) {
        discriminant = sqrt(discriminant);
        float t1 = (-b - discriminant) / (2*a);
        float t2 = (-b + discriminant) / (2*a);
        if((t1 >= 0 && t1 <= 1) || (t2 >= 0 && t2 <= 1)) return true;
    }
    return false;
}

unsigned long lastTime = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
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
    
    if (now - lastSpawn > spawnRate) {
        for(int i=0; i<MAX_FRUITS; i++) {
            if (!fruits[i].active) {
                fruits[i].active = true;
                fruits[i].x = random(20, 108);
                fruits[i].y = 64;
                fruits[i].vx = (64 - fruits[i].x) * 0.7f;
                fruits[i].vy = random(-130, -90);
                fruits[i].bomb = (random(0, 5) == 0);
                lastSpawn = now;
                if (spawnRate > 600) spawnRate *= 0.95f;
                break;
            }
        }
    }
    
    int jx = input_x() - 2048;
    int jy = input_y() - 2048;
    
    if (abs(jx) > 500) cursorX += (jx > 0 ? 1 : -1) * 150.0f * dt;
    if (abs(jy) > 500) cursorY += (jy > 0 ? 1 : -1) * 150.0f * dt;
    
    if (cursorX < 0) cursorX = 0; if (cursorX > 127) cursorX = 127;
    if (cursorY < 0) cursorY = 0; if (cursorY > 63) cursorY = 63;
    
    bool action = input_action();
    
    if (action) {
        isSlicing = true;
        for(int i=4; i>0; i--) {
            trailX[i] = trailX[i-1];
            trailY[i] = trailY[i-1];
        }
        trailX[0] = cursorX;
        trailY[0] = cursorY;
    } else {
        isSlicing = false;
        for(int i=0; i<5; i++) {
            trailX[i] = cursorX;
            trailY[i] = cursorY;
        }
    }
    
    for(int i=0; i<MAX_FRUITS; i++) {
        if (fruits[i].active) {
            fruits[i].vy += 100.0f * dt;
            fruits[i].x += fruits[i].vx * dt;
            fruits[i].y += fruits[i].vy * dt;
            
            if (fruits[i].y > 70) {
                fruits[i].active = false;
                if (!fruits[i].bomb) {
                    lives--;
                    tone(BUZZER_PIN, 200, 100);
                    if (lives <= 0) gameOver = true;
                }
            }
            
            if (isSlicing && fruits[i].active) {
                if (lineIntersectsCircle(trailX[0], trailY[0], trailX[1], trailY[1], fruits[i].x, fruits[i].y, 6.0f)) {
                    fruits[i].active = false;
                    if (fruits[i].bomb) {
                        gameOver = true;
                        tone(BUZZER_PIN, 100, 500);
                    } else {
                        score += 10;
                        tone(BUZZER_PIN, 1200, 30);
                    }
                }
            }
        }
    }
    
    display_clear();
    
    for(int i=0; i<MAX_FRUITS; i++) {
        if (fruits[i].active) {
            if (fruits[i].bomb) {
                drawCircle((int)fruits[i].x, (int)fruits[i].y, 4, 1);
                drawLine((int)fruits[i].x, (int)fruits[i].y-4, (int)fruits[i].x, (int)fruits[i].y-6, 1);
            } else {
                drawCircle((int)fruits[i].x, (int)fruits[i].y, 6, 1);
            }
        }
    }
    
    if (isSlicing) {
        for(int i=0; i<4; i++) {
            drawLine((int)trailX[i], (int)trailY[i], (int)trailX[i+1], (int)trailY[i+1], 1);
        }
    } else {
        drawLine((int)cursorX-2, (int)cursorY, (int)cursorX+2, (int)cursorY, 1);
        drawLine((int)cursorX, (int)cursorY-2, (int)cursorX, (int)cursorY+2, 1);
    }
    
    drawText(2, 2, score);
    for(int i=0; i<lives; i++) drawText(118 - i*10, 2, "X");
    
    display_render();
}

}
