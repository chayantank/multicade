#include "stacker_main.h"
#include "stacker_display.h"
#include "stacker_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Stacker {

struct Block {
    float x;
    int w;
};

const int MAX_BLOCKS = 20; 
Block blocks[MAX_BLOCKS];
int currentBlock = 0;

float moveX = 0;
float moveSpeed = 40.0f;
int moveDir = 1;

int score = 0;
bool gameOver = false;
bool win = false;

struct PowerUp {
    float x;
    int yLayer;
    bool active;
};
PowerUp powerup = {0, 0, false};
int slowTimer = 0;

void resetGame() {
    currentBlock = 0;
    blocks[0].x = 44;
    blocks[0].w = 40;
    moveX = 44;
    moveSpeed = 40.0f;
    moveDir = 1;
    score = 0;
    slowTimer = 0;
    powerup.active = false;
    gameOver = false;
    win = false;
}

void setup() {
    display_setup();
    input_setup();
    resetGame();
}

unsigned long lastTime = 0;
float sineTime = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    if (gameOver || win) {
        display_clear();
        drawText(30, 20, win ? "YOU WIN!" : "GAME OVER");
        drawText(30, 30, "SCORE:");
        drawText(70, 30, score);
        drawText(15, 50, "PRESS TO RESTART");
        display_render();
        if (input_action() && now % 500 < 50) {
            tone(BUZZER_PIN, 800, 100); delay(200);
            resetGame();
        }
        return;
    }
    
    sineTime += dt;
    
    float activeSpeed = moveSpeed;
    if (slowTimer > 0) activeSpeed *= 0.5f; 
    
    float wobble = 0;
    if (currentBlock > 8) {
        wobble = sin(sineTime * 5.0f) * 20.0f * dt;
    } else if (currentBlock > 14) {
        wobble = sin(sineTime * 10.0f) * 40.0f * dt;
    }
    
    moveX += (activeSpeed * moveDir * dt) + wobble;
    
    if (moveX <= 0) {
        moveX = 0; moveDir = 1; tone(BUZZER_PIN, 200, 20);
    }
    if (moveX + blocks[currentBlock].w >= 128) {
        moveX = 128 - blocks[currentBlock].w; moveDir = -1; tone(BUZZER_PIN, 200, 20);
    }
    
    if (!powerup.active && random(0, 100) < 2 && currentBlock < 15) {
        powerup.active = true;
        powerup.x = random(20, 108);
        powerup.yLayer = currentBlock + random(2, 5);
    }
    
    if (input_action()) {
        blocks[currentBlock].x = moveX;
        
        if (currentBlock > 0) {
            float prevX = blocks[currentBlock-1].x;
            float prevW = blocks[currentBlock-1].w;
            float curX = blocks[currentBlock].x;
            float curW = blocks[currentBlock].w;
            
            if (curX + curW < prevX || curX > prevX + prevW) {
                gameOver = true; tone(BUZZER_PIN, 100, 500);
            } else {
                float newX = max(curX, prevX);
                float newW = min(curX + curW, prevX + prevW) - newX;
                
                blocks[currentBlock].x = newX;
                blocks[currentBlock].w = newW;
                
                score += currentBlock * 10;
                
                if (powerup.active && powerup.yLayer == currentBlock) {
                    if (powerup.x >= newX && powerup.x <= newX + newW) {
                        slowTimer = 3; 
                        score += 50;
                        tone(BUZZER_PIN, 2000, 100); delay(100); tone(BUZZER_PIN, 2500, 100);
                    }
                    powerup.active = false;
                }
                
                if (currentBlock >= MAX_BLOCKS - 1) {
                    win = true; tone(BUZZER_PIN, 1000, 200); delay(200); tone(BUZZER_PIN, 1500, 400);
                } else {
                    currentBlock++;
                    blocks[currentBlock].w = newW;
                    moveSpeed += 10.0f;
                    moveX = 0; moveDir = 1;
                    if (slowTimer > 0) slowTimer--;
                    tone(BUZZER_PIN, 800, 50);
                }
            }
        } else {
            currentBlock++;
            blocks[currentBlock].w = blocks[0].w;
            moveSpeed += 10.0f;
            moveX = 0; moveDir = 1;
            score += 10;
            tone(BUZZER_PIN, 800, 50);
        }
    }
    
    display_clear();
    
    int cameraY = 0;
    if (currentBlock > 5) cameraY = (currentBlock - 5) * 6;
    
    for(int i=0; i<currentBlock; i++) {
        int y = 58 - i*6 + cameraY;
        if (y > -10 && y < 64) {
            drawRect((int)blocks[i].x, y, blocks[i].w, 6, 1);
            fillRect((int)blocks[i].x+1, y+1, blocks[i].w-2, 4, 1);
        }
    }
    
    int currentY = 58 - currentBlock*6 + cameraY;
    drawRect((int)moveX, currentY, blocks[currentBlock].w, 6, 1);
    
    if (powerup.active) {
        int py = 58 - powerup.yLayer*6 + cameraY;
        if (py > -10 && py < 64) {
            drawRect((int)powerup.x - 2, py + 1, 4, 4, 1);
            fillRect((int)powerup.x, py, 1, 7, 1);
            fillRect((int)powerup.x-3, py+3, 7, 1, 1);
        }
    }
    
    drawText(2, 2, score);
    if (slowTimer > 0) drawText(60, 2, "SLOW!");
    if (currentBlock > 8) drawText(100, 2, "WOB!");
    
    display_render();
}

}
