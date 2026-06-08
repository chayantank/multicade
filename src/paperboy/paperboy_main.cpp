#include "paperboy_main.h"
#include "paperboy_display.h"
#include "paperboy_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Paperboy {

const int SCREEN_W = 128;
const int SCREEN_H = 64;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;
unsigned long stateTimer = 0;

int score = 0;
int lives = 3;
float scrollSpeed = 30.0f;
float distance = 0;

struct Player {
    float x, y;
    int animState;
};

Player p;

struct Paper {
    float x, y;
    float vx, vy;
    bool active;
};

const int MAX_PAPERS = 3;
Paper papers[MAX_PAPERS];

struct House {
    float y;
    bool subscriber; // Red vs Blue house
    bool delivered;
    bool active;
};

const int MAX_HOUSES = 4;
House houses[MAX_HOUSES];

struct Obstacle {
    float x, y;
    float vy; // moving relative to road
    int type; // 1=car, 2=dog, 3=hydrant
    bool active;
};

const int MAX_OBS = 4;
Obstacle obs[MAX_OBS];

void spawnHouse(int i, float y) {
    houses[i].active = true;
    houses[i].y = y;
    houses[i].subscriber = (random(0, 3) > 0); // 2/3 chance to be a subscriber
    houses[i].delivered = false;
}

void spawnObstacle(int i, float y) {
    obs[i].active = true;
    obs[i].y = y;
    obs[i].type = random(1, 4);
    if (obs[i].type == 1) {
        // Car
        obs[i].x = random(50, 110);
        obs[i].vy = random(10, 40); // Drives down
    } else if (obs[i].type == 2) {
        // Dog
        obs[i].x = random(20, 40);
        obs[i].vy = random(-10, 10);
    } else {
        // Hydrant
        obs[i].x = random(35, 45); // Edge of sidewalk
        obs[i].vy = 0;
    }
}

void initLevel() {
    p.x = 50.0f;
    p.y = 50.0f;
    
    for(int i=0; i<MAX_PAPERS; i++) papers[i].active = false;
    for(int i=0; i<MAX_HOUSES; i++) spawnHouse(i, -i * 60.0f);
    for(int i=0; i<MAX_OBS; i++) spawnObstacle(i, -30.0f - i * 50.0f);
    
    scrollSpeed = 40.0f;
    distance = 0;
}

void resetGame() {
    score = 0;
    lives = 3;
    initLevel();
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

unsigned long lastTime = 0;
unsigned long animTimer = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    bool throwBtn = input_throw();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(30, 20, "PAPER ROUTE");
        drawText(20, 40, "PRESS TO START");
        display_render();
        
        if (throwBtn) {
            resetGame();
            state = STATE_PLAYING;
        }
    } else if (state == STATE_PLAYING) {
        
        // Input
        if (input_left()) p.x -= 40.0f * dt;
        if (input_right()) p.x += 40.0f * dt;
        if (input_up()) p.y -= 30.0f * dt;
        if (input_down()) p.y += 30.0f * dt;
        
        if (p.x < 10) p.x = 10;
        if (p.x > SCREEN_W - 10) p.x = SCREEN_W - 10;
        if (p.y < 10) p.y = 10;
        if (p.y > SCREEN_H - 10) p.y = SCREEN_H - 10;
        
        if (now - animTimer > 100) {
            animTimer = now;
            p.animState = (p.animState + 1) % 2;
        }
        
        // Speed up over time
        distance += scrollSpeed * dt;
        scrollSpeed += 1.0f * dt;
        if (scrollSpeed > 80.0f) scrollSpeed = 80.0f;
        
        // Throw paper
        if (throwBtn) {
            for(int i=0; i<MAX_PAPERS; i++) {
                if (!papers[i].active) {
                    papers[i].active = true;
                    papers[i].x = p.x - 5;
                    papers[i].y = p.y - 5;
                    papers[i].vx = -60.0f; // Throws diagonally up-left
                    papers[i].vy = -80.0f;
                    tone(BUZZER_PIN, 1500, 30);
                    break;
                }
            }
        }
        
        // Update papers
        for(int i=0; i<MAX_PAPERS; i++) {
            if (papers[i].active) {
                papers[i].x += papers[i].vx * dt;
                papers[i].y += (papers[i].vy + scrollSpeed) * dt; // Add scroll speed
                
                if (papers[i].x < 0 || papers[i].y < 0 || papers[i].y > SCREEN_H) {
                    papers[i].active = false;
                }
                
                // Check house collision
                for(int h=0; h<MAX_HOUSES; h++) {
                    if (houses[h].active && houses[h].subscriber && !houses[h].delivered) {
                        float mailboxX = 30.0f;
                        float mailboxY = houses[h].y + 10.0f;
                        if (abs(papers[i].x - mailboxX) < 6 && abs(papers[i].y - mailboxY) < 6) {
                            houses[h].delivered = true;
                            papers[i].active = false;
                            score += 100;
                            tone(BUZZER_PIN, 2000, 100);
                        }
                    }
                }
            }
        }
        
        // Update Houses
        for(int i=0; i<MAX_HOUSES; i++) {
            if (houses[i].active) {
                houses[i].y += scrollSpeed * dt;
                if (houses[i].y > SCREEN_H) {
                    if (houses[i].subscriber && !houses[i].delivered) {
                        // Missed delivery penalty?
                        score -= 10;
                        if (score < 0) score = 0;
                    }
                    spawnHouse(i, -40.0f); // Wrap to top
                }
            }
        }
        
        bool isHit = false;
        
        // Update Obstacles
        for(int i=0; i<MAX_OBS; i++) {
            if (obs[i].active) {
                obs[i].y += (scrollSpeed + obs[i].vy) * dt;
                
                // Dog follows player slightly
                if (obs[i].type == 2) {
                    if (p.x > obs[i].x) obs[i].x += 10.0f * dt;
                    else obs[i].x -= 10.0f * dt;
                }
                
                if (obs[i].y > SCREEN_H) {
                    spawnObstacle(i, -20.0f);
                }
                
                // Player collision
                float ow = (obs[i].type == 1) ? 12 : 6;
                float oh = (obs[i].type == 1) ? 16 : 6;
                
                if (abs(p.x - obs[i].x) < ow && abs(p.y - obs[i].y) < oh) {
                    isHit = true;
                }
            }
        }
        
        if (isHit) {
            lives--;
            tone(BUZZER_PIN, 100, 500);
            if (lives <= 0) {
                state = STATE_GAMEOVER;
                stateTimer = now;
            } else {
                p.x = 80.0f;
                p.y = 50.0f;
                for(int i=0; i<MAX_OBS; i++) {
                    if (obs[i].y > 0 && obs[i].y < SCREEN_H) obs[i].y = -100.0f; // Clear screen
                }
            }
        }
        
        // Render
        display_clear();
        
        // Environment (Road lines)
        int yOffset = (int)distance % 20;
        for(int y = yOffset - 20; y < SCREEN_H; y += 20) {
            drawLine(40, y, 40, y+10, 1); // Left curb
            drawLine(120, y, 120, y+10, 1); // Right curb
            drawLine(80, y, 80, y+5, 1); // Center line
        }
        
        // Houses
        for(int i=0; i<MAX_HOUSES; i++) {
            if (houses[i].active) {
                int hx = 2;
                int hy = (int)houses[i].y;
                // House shape
                drawRect(hx, hy, 20, 20, 1);
                drawLine(hx, hy, hx+10, hy-10, 1); // Roof
                drawLine(hx+10, hy-10, hx+20, hy, 1);
                // Windows/Door
                drawRect(hx+4, hy+10, 6, 10, 1);
                
                // Color/Style based on subscriber
                if (houses[i].subscriber) {
                    if (houses[i].delivered) fillRect(hx+2, hy+2, 16, 6, 1); // Dark windows
                    else drawRect(hx+2, hy+2, 16, 6, 1);
                    
                    // Mailbox
                    drawRect(30, hy+8, 4, 4, 1);
                    drawLine(32, hy+12, 32, hy+16, 1);
                } else {
                    // Non subscriber (darker house)
                    for(int l=0; l<20; l+=4) drawLine(hx, hy+l, hx+20, hy+l, 1);
                }
            }
        }
        
        // Obstacles
        for(int i=0; i<MAX_OBS; i++) {
            if (obs[i].active) {
                int ox = (int)obs[i].x;
                int oy = (int)obs[i].y;
                if (obs[i].type == 1) {
                    // Car
                    drawRect(ox-6, oy-8, 12, 16, 1);
                    fillRect(ox-4, oy-4, 8, 8, 1);
                } else if (obs[i].type == 2) {
                    // Dog
                    drawRect(ox-3, oy-2, 6, 4, 1);
                    drawPixel(ox-4, oy-3, 1); // Head
                } else {
                    // Hydrant
                    fillRect(ox-2, oy-4, 4, 8, 1);
                    drawLine(ox-4, oy-1, ox+4, oy-1, 1);
                }
            }
        }
        
        // Papers
        for(int i=0; i<MAX_PAPERS; i++) {
            if (papers[i].active) {
                int px = (int)papers[i].x;
                int py = (int)papers[i].y;
                drawLine(px-2, py-2, px+2, py+2, 1);
                drawLine(px+2, py-2, px-2, py+2, 1);
            }
        }
        
        // Player (Bike)
        int px = (int)p.x;
        int py = (int)p.y;
        drawLine(px, py-6, px, py+6, 1); // Body
        drawCircle(px, py-6, 2, 1); // Front tire
        drawCircle(px, py+6, 2, 1); // Back tire
        // Legs
        if (p.animState == 0) drawLine(px-3, py, px+3, py, 1);
        else drawLine(px-2, py+2, px+2, py-2, 1);
        
        // HUD
        fillRect(0, 0, SCREEN_W, 8, 0);
        drawText(0, 0, "S:"); drawText(15, 0, score);
        for(int i=0; i<lives; i++) drawRect(100 + i*6, 1, 4, 4, 1);
        
        display_render();
        
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(35, 20, "GAME OVER");
        drawText(20, 40, "SCORE:");
        drawText(60, 40, score);
        display_render();
        
        if (throwBtn && now - stateTimer > 1000) {
            state = STATE_INTRO;
        }
    }
}

}
