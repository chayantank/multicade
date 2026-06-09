#include "dino_main.h"
#include "dino_display.h"
#include "dino_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Dino {

const int GROUND_Y = 50;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;

float dinoY = 0;
float dinoVY = 0;
bool isJumping = false;
bool isDucking = false;

float gameSpeed = 40.0f;
float distance = 0;
int highScore = 0;
int lastMilestone = 0;

struct Obstacle {
    float x;
    int type; // 0=cactus small, 1=cactus large, 2=cactus group, 3=bird low, 4=bird high, 5=star
    bool active;
};

const int MAX_OBSTACLES = 4;
Obstacle obs[MAX_OBSTACLES];

void spawnObstacle() {
    for(int i=0; i<MAX_OBSTACLES; i++) {
        if (!obs[i].active) {
            obs[i].active = true;
            obs[i].x = 128 + random(0, 50);
            
            if (random(0, 100) < 15) {
                obs[i].type = 5; // Star
                return;
            }
            
            if (gameSpeed < 50) obs[i].type = random(0, 2);
            else if (gameSpeed < 65) obs[i].type = random(0, 3);
            else obs[i].type = random(0, 5);
            return;
        }
    }
}

void resetGame() {
    dinoY = GROUND_Y - 24;
    dinoVY = 0;
    isJumping = false;
    isDucking = false;
    gameSpeed = 40.0f;
    distance = 0;
    lastMilestone = 0;
    for(int i=0; i<MAX_OBSTACLES; i++) obs[i].active = false;
    state = STATE_PLAYING;
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

unsigned long lastTime = 0;
unsigned long lastSpawnTime = 0;
unsigned long stateTimer = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    if (state == STATE_INTRO) {
        display_clear();
        drawRect(0, 0, 128, 64, 1);
        drawRect(2, 2, 124, 60, 1);
        drawText(35, 20, "DINO DASH");
        
        if ((now / 500) % 2 == 0) {
            drawText(20, 45, "CLICK TO START");
        }
        display_render();
        
        if (input_jump()) {
            tone(BUZZER_PIN, 800, 100);
            delay(200);
            resetGame();
        }
        return;
    }
    
    if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(35, 20, "GAME OVER");
        drawText(40, 30, (int)distance);
        drawText(15, 45, "CLICK TO RESTART");
        display_render();
        
        if (input_jump() && now - stateTimer > 500) {
            tone(BUZZER_PIN, 800, 100);
            delay(200);
            resetGame();
        }
        return;
    }
    
    // PLAYING state
    isDucking = input_duck() && !isJumping;
    
    if (input_jump() && !isJumping) {
        isJumping = true;
        dinoVY = -180.0f;
        tone(BUZZER_PIN, 600, 50);
    }
    
    if (isJumping && input_duck()) {
        dinoVY += 400.0f * dt; 
    }
    
    if (isJumping) {
        dinoVY += 600.0f * dt; 
        dinoY += dinoVY * dt;
        
        if (dinoY >= GROUND_Y - 24) {
            dinoY = GROUND_Y - 24;
            isJumping = false;
            dinoVY = 0;
        }
    }
    
    gameSpeed += 0.5f * dt;
    distance += (gameSpeed / 10.0f) * dt;
    if (distance > highScore) highScore = distance;
    
    int currentMilestone = (int)distance / 100;
    if (currentMilestone > lastMilestone) {
        lastMilestone = currentMilestone;
        tone(BUZZER_PIN, 1500, 200);
    }
    
    if (now - lastSpawnTime > (200000.0f / gameSpeed)) {
        spawnObstacle();
        lastSpawnTime = now;
    }
    
    for(int i=0; i<MAX_OBSTACLES; i++) {
        if (obs[i].active) {
            obs[i].x -= gameSpeed * dt;
            if (obs[i].x < -30) {
                obs[i].active = false;
                if (obs[i].type != 5) tone(BUZZER_PIN, 1000, 20); 
            }
            
            int dx = 20;
            int dy = (int)dinoY;
            int dw = 16;
            int dh = isDucking ? 16 : 24;
            if (isDucking) dy += 8;
            
            int ox = (int)obs[i].x;
            int oy = 0;
            int ow = 0;
            int oh = 0;
            
            if (obs[i].type == 0) { oy = GROUND_Y - 16; ow = 8; oh = 16; }
            else if (obs[i].type == 1) { oy = GROUND_Y - 24; ow = 12; oh = 24; }
            else if (obs[i].type == 2) { oy = GROUND_Y - 24; ow = 26; oh = 24; }
            else if (obs[i].type == 3) { oy = GROUND_Y - 18; ow = 16; oh = 8; }
            else if (obs[i].type == 4) { oy = GROUND_Y - 32; ow = 16; oh = 8; }
            else if (obs[i].type == 5) { oy = GROUND_Y - 45; ow = 8; oh = 8; } 
            
            if (dx < ox + ow && dx + dw > ox && dy < oy + oh && dy + dh > oy) {
                if (obs[i].type == 5) {
                    obs[i].active = false;
                    distance += 25;
                    tone(BUZZER_PIN, 2000, 100);
                } else {
                    state = STATE_GAMEOVER;
                    stateTimer = now;
                    tone(BUZZER_PIN, 200, 500);
                }
            }
        }
    }
    
    display_clear();
    
    drawLine(0, GROUND_Y, 128, GROUND_Y, 1);
    
    for(int i=0; i<3; i++) {
        int cx = (int)(distance * -20.0f + i*60) % 128;
        if (cx < 0) cx += 128;
        drawRect(cx, 10 + i*5, 10, 2, 1);
    }
    
    bool drawScore = true;
    if (lastMilestone > 0 && distance - (lastMilestone * 100) < 5) {
        if ((now / 100) % 2 == 0) drawScore = false;
    }
    
    if (drawScore) {
        drawText(80, 2, (int)distance);
    }
    drawText(2, 2, "HI:");
    drawText(20, 2, highScore);
    
    int animFrame = (int)(distance * 2) % 2;
    if (isJumping) animFrame = 0;
    drawDino(20, (int)dinoY, animFrame, isDucking);
    
    for(int i=0; i<MAX_OBSTACLES; i++) {
        if (obs[i].active) {
            if (obs[i].type <= 2) {
                drawCactus((int)obs[i].x, GROUND_Y - (obs[i].type == 0 ? 16 : 24), obs[i].type);
            } else if (obs[i].type <= 4) {
                int by = GROUND_Y - (obs[i].type == 3 ? 18 : 32);
                drawBird((int)obs[i].x, by, animFrame);
            } else if (obs[i].type == 5) {
                int sx = (int)obs[i].x;
                int sy = GROUND_Y - 45;
                drawLine(sx, sy+3, sx+6, sy+3, 1);
                drawLine(sx+3, sy, sx+3, sy+6, 1);
                drawLine(sx+1, sy+1, sx+5, sy+5, 1);
                drawLine(sx+1, sy+5, sx+5, sy+1, 1);
            }
        }
    }
    
    display_render();
}

}
