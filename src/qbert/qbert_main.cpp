#include "qbert_main.h"
#include "qbert_display.h"
#include "qbert_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Qbert {

const int PYRAMID_ROWS = 6;
int tiles[PYRAMID_ROWS][PYRAMID_ROWS]; // r, c. Valid if c <= r

int playerR = 0;
int playerC = 0;
float jumpProgress = 0; // 0 to 1
int jumpDirR = 0;
int jumpDirC = 0;

struct Enemy {
    int r, c;
    float jumpProgress;
    int jumpDirR, jumpDirC;
    bool active;
};

const int MAX_ENEMIES = 3;
Enemy enemies[MAX_ENEMIES];

int score = 0;
int lives = 3;
int level = 1;
bool gameOver = false;
unsigned long lastEnemySpawn = 0;

void nextLevel() {
    for(int r=0; r<PYRAMID_ROWS; r++) {
        for(int c=0; c<=r; c++) {
            tiles[r][c] = 0;
        }
    }
    playerR = 0; playerC = 0;
    jumpProgress = 0;
    for(int i=0; i<MAX_ENEMIES; i++) enemies[i].active = false;
    tone(BUZZER_PIN, 1000, 100); delay(150); tone(BUZZER_PIN, 1500, 200);
}

void resetGame() {
    score = 0;
    lives = 3;
    level = 1;
    gameOver = false;
    nextLevel();
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
        drawText(35, 20, "GAME OVER");
        drawText(40, 30, score);
        display_render();
        if (input_upLeft()) {
            tone(BUZZER_PIN, 800, 100); delay(200);
            resetGame();
        }
        return;
    }
    
    // Check level complete
    bool complete = true;
    for(int r=0; r<PYRAMID_ROWS; r++) {
        for(int c=0; c<=r; c++) {
            if (tiles[r][c] == 0) complete = false;
        }
    }
    if (complete) {
        level++;
        score += 1000;
        nextLevel();
        return;
    }
    
    // Player Input
    if (jumpProgress == 0) {
        bool jumping = false;
        if (input_upLeft()) { jumpDirR = -1; jumpDirC = -1; jumping = true; }
        else if (input_upRight()) { jumpDirR = -1; jumpDirC = 0; jumping = true; }
        else if (input_downLeft()) { jumpDirR = 1; jumpDirC = 0; jumping = true; }
        else if (input_downRight()) { jumpDirR = 1; jumpDirC = 1; jumping = true; }
        
        if (jumping) {
            jumpProgress = 0.01f;
            tone(BUZZER_PIN, 400, 30);
        }
    } else {
        jumpProgress += 5.0f * dt;
        if (jumpProgress >= 1.0f) {
            jumpProgress = 0;
            playerR += jumpDirR;
            playerC += jumpDirC;
            
            // Check falling off
            if (playerR < 0 || playerR >= PYRAMID_ROWS || playerC < 0 || playerC > playerR) {
                // Fell off
                lives--;
                tone(BUZZER_PIN, 200, 500);
                if (lives <= 0) gameOver = true;
                else {
                    playerR = 0; playerC = 0;
                }
            } else {
                // Landed safely
                if (tiles[playerR][playerC] == 0) {
                    tiles[playerR][playerC] = 1;
                    score += 25;
                    tone(BUZZER_PIN, 800, 50);
                }
            }
        }
    }
    
    // Enemies
    if (now - lastEnemySpawn > (3000.0f / (1.0f + level*0.2f))) {
        for(int i=0; i<MAX_ENEMIES; i++) {
            if (!enemies[i].active) {
                enemies[i].active = true;
                enemies[i].r = 0; enemies[i].c = 0;
                enemies[i].jumpProgress = 0;
                lastEnemySpawn = now;
                break;
            }
        }
    }
    
    for(int i=0; i<MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            if (enemies[i].jumpProgress == 0) {
                if (random(0, 100) < 2) {
                    enemies[i].jumpProgress = 0.01f;
                    enemies[i].jumpDirR = 1;
                    enemies[i].jumpDirC = random(0, 2); // only down-left or down-right
                }
            } else {
                enemies[i].jumpProgress += 4.0f * dt;
                if (enemies[i].jumpProgress >= 1.0f) {
                    enemies[i].jumpProgress = 0;
                    enemies[i].r += enemies[i].jumpDirR;
                    enemies[i].c += enemies[i].jumpDirC;
                    if (enemies[i].r >= PYRAMID_ROWS) {
                        enemies[i].active = false; // fell off bottom
                    }
                }
            }
            
            // Check collision with player
            if (enemies[i].r == playerR && enemies[i].c == playerC) {
                lives--;
                tone(BUZZER_PIN, 150, 400);
                enemies[i].active = false;
                if (lives <= 0) gameOver = true;
                else {
                    playerR = 0; playerC = 0;
                    jumpProgress = 0;
                }
            }
        }
    }
    
    // Render
    display_clear();
    
    // Draw pyramid from top to bottom
    for(int r=0; r<PYRAMID_ROWS; r++) {
        for(int c=0; c<=r; c++) {
            int sx, sy;
            getScreenPos(r, c, sx, sy);
            drawCube(sx, sy, tiles[r][c]);
            
            // Draw entities on this tile
            // Enemies
            for(int i=0; i<MAX_ENEMIES; i++) {
                if (enemies[i].active && enemies[i].r == r && enemies[i].c == c && enemies[i].jumpProgress == 0) {
                    drawEnemy(sx, sy, 0);
                } else if (enemies[i].active && enemies[i].jumpProgress > 0) {
                    // Mid jump
                    if (enemies[i].r == r && enemies[i].c == c) {
                        // leaving this tile
                        int nx, ny;
                        getScreenPos(r + enemies[i].jumpDirR, c + enemies[i].jumpDirC, nx, ny);
                        int ex = sx + (nx - sx) * enemies[i].jumpProgress;
                        int ey = sy + (ny - sy) * enemies[i].jumpProgress;
                        float hop = sin(enemies[i].jumpProgress * 3.14f) * 10.0f;
                        drawEnemy(ex - 8, ey + 4 - hop, hop);
                    }
                }
            }
            
            // Player
            if (playerR == r && playerC == c && jumpProgress == 0) {
                drawPlayer(sx, sy, 0);
            } else if (jumpProgress > 0) {
                if (playerR == r && playerC == c) {
                    // leaving this tile
                    int nx, ny;
                    getScreenPos(r + jumpDirR, c + jumpDirC, nx, ny);
                    int px = sx + (nx - sx) * jumpProgress;
                    int py = sy + (ny - sy) * jumpProgress;
                    float hop = sin(jumpProgress * 3.14f) * 12.0f;
                    drawPlayer(px - 8, py + 4 - hop, hop);
                }
            }
        }
    }
    
    // UI
    drawText(2, 2, score);
    drawText(110, 2, "L:");
    drawText(120, 2, lives);
    
    display_render();
}

}
