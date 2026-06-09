#include "slingshot_main.h"
#include "slingshot_display.h"
#include "slingshot_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Slingshot {

struct Block {
    float x, y;
    float w, h;
    float vx, vy;
    bool active;
    bool isPig;
    bool isTNT;
};

const int MAX_BLOCKS = 15;
Block blocks[MAX_BLOCKS];

struct Bird {
    float x, y;
    float vx, vy;
    bool active;
    bool flying;
};

Bird bird;

enum State {
    STATE_INTRO,
    AIMING,
    FLYING,
    SIMULATING,
    GAME_OVER,
    LEVEL_CLEAR
};

State state;
float slingX = 20;
float slingY = 50;
float pullX = 20;
float pullY = 50;
int shotsLeft = 3;
int level = 1;

void loadLevel() {
    for(int i=0; i<MAX_BLOCKS; i++) blocks[i].active = false;
    
    if (level == 1) {
        blocks[0] = {90, 54, 10, 10, 0, 0, true, false, false};
        blocks[1] = {90, 44, 10, 10, 0, 0, true, false, false};
        blocks[2] = {92, 38, 6, 6, 0, 0, true, true, false};
    } else if (level == 2) {
        blocks[0] = {80, 54, 10, 10, 0, 0, true, false, false};
        blocks[1] = {100, 54, 10, 10, 0, 0, true, false, false};
        blocks[2] = {80, 44, 30, 10, 0, 0, true, false, false};
        blocks[3] = {88, 56, 6, 6, 0, 0, true, true, false};
        blocks[4] = {90, 34, 10, 10, 0, 0, true, false, true}; // TNT on top
    } else {
        blocks[0] = {70, 54, 8, 10, 0, 0, true, false, false};
        blocks[1] = {90, 54, 8, 10, 0, 0, true, false, true}; // TNT inside
        blocks[2] = {110, 54, 8, 10, 0, 0, true, false, false};
        blocks[3] = {70, 44, 48, 10, 0, 0, true, false, false};
        blocks[4] = {80, 34, 10, 10, 0, 0, true, false, false};
        blocks[5] = {100, 34, 10, 10, 0, 0, true, false, false};
        blocks[6] = {80, 56, 6, 6, 0, 0, true, true, false};
        blocks[7] = {100, 56, 6, 6, 0, 0, true, true, false};
        blocks[8] = {90, 28, 6, 6, 0, 0, true, true, false};
    }
    
    state = AIMING;
    shotsLeft = 3;
    bird.active = true;
    bird.flying = false;
    bird.x = slingX;
    bird.y = slingY;
}

void resetGame() {
    level = 1;
    loadLevel();
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

bool checkCollision(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
    return (ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by);
}

void explodeTNT(int tntIndex) {
    blocks[tntIndex].active = false;
    float tx = blocks[tntIndex].x + blocks[tntIndex].w/2;
    float ty = blocks[tntIndex].y + blocks[tntIndex].h/2;
    
    for(int i=0; i<MAX_BLOCKS; i++) {
        if (blocks[i].active) {
            float cx = blocks[i].x + blocks[i].w/2;
            float cy = blocks[i].y + blocks[i].h/2;
            float dx = cx - tx;
            float dy = cy - ty;
            float dist = sqrt(dx*dx + dy*dy);
            if (dist < 40) {
                blocks[i].vx += (dx / dist) * 200.0f;
                blocks[i].vy += (dy / dist) * 200.0f - 50.0f;
            }
        }
    }
    tone(BUZZER_PIN, 100, 400); // Explosion sound
}

unsigned long lastTime = 0;
unsigned long simStartTime = 0;
unsigned long stateTimer = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.05f) dt = 0.05f;
    
    if (state == STATE_INTRO) {
        display_clear();
        drawRect(0, 0, 128, 64, 1);
        drawRect(2, 2, 124, 60, 1);
        drawText(35, 20, "SLINGSHOT");
        
        if ((now / 500) % 2 == 0) {
            drawText(20, 45, "CLICK TO START");
        }
        display_render();
        
        if (input_action()) {
            tone(BUZZER_PIN, 800, 100);
            delay(200);
            resetGame();
        }
        return;
    }
    
    if (state == GAME_OVER) {
        display_clear();
        drawText(35, 20, "GAME OVER");
        drawText(15, 45, "CLICK TO RESTART");
        display_render();
        if (input_action() && now - simStartTime > 1000) resetGame();
        return;
    }
    
    if (state == LEVEL_CLEAR) {
        display_clear();
        drawText(30, 20, "LEVEL CLEAR");
        drawText(20, 45, "CLICK TO CONT");
        display_render();
        if (input_action() && now - simStartTime > 1000) {
            level++;
            loadLevel();
        }
        return;
    }
    
    if (state == AIMING) {
        if (input_action()) {
            int jx = input_x() - 2048;
            int jy = input_y() - 2048;
            pullX -= jx * 0.01f;
            pullY -= jy * 0.01f;
            
            float dx = pullX - slingX;
            float dy = pullY - slingY;
            float dist = sqrt(dx*dx + dy*dy);
            if (dist > 15) {
                pullX = slingX + (dx/dist) * 15;
                pullY = slingY + (dy/dist) * 15;
            }
            if (pullX > slingX) pullX = slingX;
            bird.x = pullX;
            bird.y = pullY;
        } else {
            if (pullX < slingX - 5 || pullY != slingY) {
                bird.vx = (slingX - pullX) * 15.0f;
                bird.vy = (slingY - pullY) * 15.0f;
                bird.flying = true;
                state = FLYING;
                tone(BUZZER_PIN, 800, 100);
            }
            pullX = slingX;
            pullY = slingY;
            bird.x = slingX;
            bird.y = slingY;
        }
    }
    
    if (state == FLYING || state == SIMULATING) {
        if (bird.flying) {
            bird.vy += 150.0f * dt;
            bird.x += bird.vx * dt;
            bird.y += bird.vy * dt;
            
            if (bird.y > 60) {
                bird.y = 60;
                bird.vx *= 0.5f;
                bird.vy *= -0.5f;
                if (abs(bird.vx) < 10) {
                    bird.flying = false;
                    simStartTime = now;
                    state = SIMULATING;
                }
            }
            if (bird.x > 130) {
                bird.flying = false;
                simStartTime = now;
                state = SIMULATING;
            }
        }
        
        int steps = 3;
        float subDt = dt / steps;
        for(int s=0; s<steps; s++) {
            if (bird.flying) {
                for(int i=0; i<MAX_BLOCKS; i++) {
                    if (blocks[i].active && checkCollision(bird.x-2, bird.y-2, 4, 4, blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h)) {
                        if (blocks[i].isTNT) {
                            explodeTNT(i);
                        } else {
                            blocks[i].vx += bird.vx * 0.5f;
                            blocks[i].vy += bird.vy * 0.5f;
                            bird.vx *= 0.5f;
                            bird.vy *= 0.5f;
                            if (blocks[i].isPig) {
                                blocks[i].active = false;
                                tone(BUZZER_PIN, 1200, 50);
                            } else {
                                tone(BUZZER_PIN, 200, 30);
                            }
                        }
                    }
                }
            }
            
            for(int i=0; i<MAX_BLOCKS; i++) {
                if (blocks[i].active) {
                    blocks[i].vy += 150.0f * subDt;
                    blocks[i].x += blocks[i].vx * subDt;
                    blocks[i].y += blocks[i].vy * subDt;
                    blocks[i].vx *= 0.95f;
                    
                    if (blocks[i].y + blocks[i].h > 64) {
                        blocks[i].y = 64 - blocks[i].h;
                        blocks[i].vy *= -0.3f;
                        blocks[i].vx *= 0.8f;
                        if (blocks[i].isTNT && abs(blocks[i].vy) > 40) {
                            explodeTNT(i);
                        }
                    }
                    
                    if (blocks[i].isPig && (abs(blocks[i].vx) > 30 || abs(blocks[i].vy) > 30)) {
                        blocks[i].active = false;
                        tone(BUZZER_PIN, 1200, 50);
                    }
                    
                    for(int j=0; j<MAX_BLOCKS; j++) {
                        if (i!=j && blocks[j].active) {
                            if (checkCollision(blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h, blocks[j].x, blocks[j].y, blocks[j].w, blocks[j].h)) {
                                if (blocks[i].isTNT && (abs(blocks[i].vx) > 40 || abs(blocks[i].vy) > 40 || abs(blocks[j].vx) > 40 || abs(blocks[j].vy) > 40)) {
                                    explodeTNT(i);
                                } else if (blocks[j].isTNT && (abs(blocks[i].vx) > 40 || abs(blocks[i].vy) > 40 || abs(blocks[j].vx) > 40 || abs(blocks[j].vy) > 40)) {
                                    explodeTNT(j);
                                } else {
                                    if (blocks[i].y < blocks[j].y) {
                                        blocks[i].y = blocks[j].y - blocks[i].h;
                                        blocks[i].vy = 0;
                                        blocks[j].vy = 0;
                                    } else {
                                        blocks[j].y = blocks[i].y - blocks[j].h;
                                        blocks[i].vy = 0;
                                        blocks[j].vy = 0;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    if (state == SIMULATING && now - simStartTime > 2000) {
        bool pigsLeft = false;
        for(int i=0; i<MAX_BLOCKS; i++) {
            if (blocks[i].active && blocks[i].isPig) pigsLeft = true;
        }
        
        if (!pigsLeft) {
            state = LEVEL_CLEAR;
            simStartTime = now;
        } else {
            shotsLeft--;
            if (shotsLeft <= 0) {
                state = GAME_OVER;
                simStartTime = now;
            } else {
                state = AIMING;
                bird.active = true;
                bird.flying = false;
                pullX = slingX;
                pullY = slingY;
                bird.x = slingX;
                bird.y = slingY;
            }
        }
    }
    
    display_clear();
    
    drawLine(0, 63, 127, 63, 1);
    
    if (state == AIMING) {
        drawLine(slingX, slingY, pullX, pullY, 1);
    }
    
    drawLine(slingX, 63, slingX, slingY, 1);
    
    if (bird.active) drawCircle((int)bird.x, (int)bird.y, 2, 1);
    
    for(int i=0; i<MAX_BLOCKS; i++) {
        if (blocks[i].active) {
            if (blocks[i].isPig) {
                drawCircle((int)(blocks[i].x + blocks[i].w/2), (int)(blocks[i].y + blocks[i].h/2), 3, 1);
            } else if (blocks[i].isTNT) {
                drawRect((int)blocks[i].x, (int)blocks[i].y, (int)blocks[i].w, (int)blocks[i].h, 1);
                drawLine((int)blocks[i].x, (int)blocks[i].y, (int)(blocks[i].x+blocks[i].w), (int)(blocks[i].y+blocks[i].h), 1);
                drawLine((int)(blocks[i].x+blocks[i].w), (int)blocks[i].y, (int)blocks[i].x, (int)(blocks[i].y+blocks[i].h), 1);
            } else {
                drawRect((int)blocks[i].x, (int)blocks[i].y, (int)blocks[i].w, (int)blocks[i].h, 1);
            }
        }
    }
    
    drawText(2, 2, "Shots:");
    drawText(40, 2, shotsLeft);
    drawText(100, 2, "L:");
    drawText(112, 2, level);
    
    display_render();
}

}
