#include "bounce_main.h"
#include "bounce_display.h"
#include "bounce_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Bounce {

float bx = 10, by = 10;
float vx = 0, vy = 0;
float gravity = 120.0f;
float jumpPower = -60.0f;
float bounciness = 0.8f;
float radius = 3;

struct Ring {
    float x, y;
    bool collected;
};

const int MAX_RINGS = 5;
Ring rings[MAX_RINGS];

struct Platform {
    float x, y, w, h;
    bool isSpike;
};

const int MAX_PLATFORMS = 10;
Platform platforms[MAX_PLATFORMS];

float cameraX = 0;
int level = 1;
bool gameOver = false;
bool levelClear = false;

void loadLevel() {
    if (level == 1) {
        bx = 20; by = 20;
        platforms[0] = {0, 50, 60, 14, false};
        platforms[1] = {80, 50, 60, 14, false};
        platforms[2] = {160, 40, 40, 24, false};
        platforms[3] = {220, 50, 60, 14, false};
        platforms[4] = {240, 46, 20, 4, true};
        for(int i=5; i<MAX_PLATFORMS; i++) platforms[i] = {-100, -100, 0, 0, false};
        
        rings[0] = {90, 30, false};
        rings[1] = {110, 30, false};
        rings[2] = {170, 20, false};
        rings[3] = {250, 20, false};
        for(int i=4; i<MAX_RINGS; i++) rings[i] = {-100, -100, true};
    } else {
        bx = 20; by = 20;
        platforms[0] = {0, 40, 40, 24, false};
        platforms[1] = {60, 50, 40, 14, false};
        platforms[2] = {70, 46, 20, 4, true};
        platforms[3] = {120, 30, 40, 34, false};
        platforms[4] = {180, 50, 80, 14, false};
        platforms[5] = {200, 46, 40, 4, true};
        for(int i=6; i<MAX_PLATFORMS; i++) platforms[i] = {-100, -100, 0, 0, false};
        
        rings[0] = {70, 20, false};
        rings[1] = {130, 10, false};
        rings[2] = {220, 30, false};
        for(int i=3; i<MAX_RINGS; i++) rings[i] = {-100, -100, true};
    }
    vx = 0; vy = 0;
    cameraX = 0;
    gameOver = false;
    levelClear = false;
}

void setup() {
    display_setup();
    input_setup();
    level = 1;
    loadLevel();
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
        display_render();
        if (input_action()) {
            loadLevel();
        }
        return;
    }
    
    if (levelClear) {
        display_clear();
        drawText(30, 20, "LEVEL CLEAR");
        display_render();
        if (input_action()) {
            level++;
            loadLevel();
        }
        return;
    }
    
    int jx = input_x() - 2048;
    if (abs(jx) > 500) {
        vx += (jx > 0 ? 1 : -1) * 80.0f * dt;
    } else {
        vx *= 0.95f;
    }
    
    if (vx > 60) vx = 60;
    if (vx < -60) vx = -60;
    
    vy += gravity * dt;
    
    float nextX = bx + vx * dt;
    float nextY = by + vy * dt;
    
    bool onGround = false;
    for(int i=0; i<MAX_PLATFORMS; i++) {
        if (platforms[i].w == 0) continue;
        
        float px = platforms[i].x;
        float py = platforms[i].y;
        float pw = platforms[i].w;
        float ph = platforms[i].h;
        
        if (nextX + radius > px && nextX - radius < px + pw &&
            nextY + radius > py && nextY - radius < py + ph) {
            
            if (platforms[i].isSpike) {
                gameOver = true;
                tone(BUZZER_PIN, 100, 300);
            } else {
                float overlapLeft = (nextX + radius) - px;
                float overlapRight = (px + pw) - (nextX - radius);
                float overlapTop = (nextY + radius) - py;
                float overlapBottom = (py + ph) - (nextY - radius);
                
                float minOverlap = min(min(overlapLeft, overlapRight), min(overlapTop, overlapBottom));
                
                if (minOverlap == overlapTop) {
                    nextY = py - radius;
                    vy = jumpPower;
                    onGround = true;
                    tone(BUZZER_PIN, 600, 20);
                } else if (minOverlap == overlapBottom) {
                    nextY = py + ph + radius;
                    vy = -vy * bounciness;
                } else if (minOverlap == overlapLeft) {
                    nextX = px - radius;
                    vx = -vx * bounciness;
                } else if (minOverlap == overlapRight) {
                    nextX = px + pw + radius;
                    vx = -vx * bounciness;
                }
            }
        }
    }
    
    bx = nextX;
    by = nextY;
    
    if (by > 80) {
        gameOver = true;
        tone(BUZZER_PIN, 100, 300);
    }
    
    bool allCollected = true;
    for(int i=0; i<MAX_RINGS; i++) {
        if (!rings[i].collected) {
            float dx = rings[i].x - bx;
            float dy = rings[i].y - by;
            if (sqrt(dx*dx + dy*dy) < radius + 4) {
                rings[i].collected = true;
                tone(BUZZER_PIN, 1200, 50);
            } else {
                allCollected = false;
            }
        }
    }
    
    if (allCollected) {
        levelClear = true;
        tone(BUZZER_PIN, 800, 100); delay(100); tone(BUZZER_PIN, 1200, 100);
    }
    
    float targetCam = bx - 64;
    cameraX += (targetCam - cameraX) * 5.0f * dt;
    if (cameraX < 0) cameraX = 0;
    
    display_clear();
    
    for(int i=0; i<MAX_PLATFORMS; i++) {
        if (platforms[i].w > 0) {
            int drawX = (int)(platforms[i].x - cameraX);
            if (drawX + platforms[i].w > 0 && drawX < 128) {
                if (platforms[i].isSpike) {
                    for(int sx = 0; sx < platforms[i].w; sx += 4) {
                        drawLine(drawX + sx, platforms[i].y + platforms[i].h, drawX + sx + 2, platforms[i].y, 1);
                        drawLine(drawX + sx + 4, platforms[i].y + platforms[i].h, drawX + sx + 2, platforms[i].y, 1);
                    }
                } else {
                    drawRect(drawX, platforms[i].y, platforms[i].w, platforms[i].h, 1);
                }
            }
        }
    }
    
    for(int i=0; i<MAX_RINGS; i++) {
        if (!rings[i].collected) {
            int drawX = (int)(rings[i].x - cameraX);
            drawCircle(drawX, rings[i].y, 4, 1);
            drawCircle(drawX, rings[i].y, 2, 1);
        }
    }
    
    drawCircle((int)(bx - cameraX), (int)by, radius, 1);
    
    drawText(2, 2, "LVL"); drawText(24, 2, level);
    
    display_render();
}

}
