#include "platformer_main.h"
#include "platformer_display.h"
#include "platformer_input.h"

#define BUZZER_PIN 14

namespace Platformer {

const int SCREEN_W = 128;
const int SCREEN_H = 64;
const int TILE_SIZE = 8;
const int LEVEL_W = 64;
const int LEVEL_H = 8;

// 0=Air, 1=Wall, 2=Spike, 3=Coin, 4=Goal
const uint8_t levelData[LEVEL_W * LEVEL_H] = {
    // Row 0
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    // Row 1
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    // Row 2
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    // Row 3
    0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    // Row 4
    0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
    // Row 5
    0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,3,0,0,0,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,4,0,0,
    // Row 6
    1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,0,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,
    // Row 7
    1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,1,2,2,2,1,1,1,2,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1,1,1,1
};

uint8_t currentLevel[LEVEL_W * LEVEL_H];

float px = 8.0f;
float py = 32.0f;
float vx = 0.0f;
float vy = 0.0f;
const float GRAVITY = 800.0f;
const float JUMP_POWER = -220.0f;
const float RUN_SPEED = 80.0f;
const float RUN_ACCEL = 400.0f;
const float RUN_FRICTION = 600.0f;

bool grounded = false;
bool levelComplete = false;
int coins = 0;

int cameraX = 0;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_WIN
};

GameState state = STATE_INTRO;

void resetLevel() {
    memcpy(currentLevel, levelData, LEVEL_W * LEVEL_H);
    px = 8.0f;
    py = 32.0f;
    vx = 0.0f;
    vy = 0.0f;
    coins = 0;
    cameraX = 0;
    levelComplete = false;
}

uint8_t getTile(int tx, int ty) {
    if (tx < 0 || tx >= LEVEL_W || ty < 0 || ty >= LEVEL_H) return 1; // Walls outside
    return currentLevel[ty * LEVEL_W + tx];
}

void setTile(int tx, int ty, uint8_t v) {
    if (tx < 0 || tx >= LEVEL_W || ty < 0 || ty >= LEVEL_H) return;
    currentLevel[ty * LEVEL_W + tx] = v;
}

// AABB Collision vs Tilemap
bool checkCollision(float cx, float cy, int& hitType, int& hitTx, int& hitTy) {
    // Player hitbox: 6x6 centered roughly
    int left = (int)(cx + 1) / TILE_SIZE;
    int right = (int)(cx + 6) / TILE_SIZE;
    int top = (int)(cy + 1) / TILE_SIZE;
    int bottom = (int)(cy + 7) / TILE_SIZE; // Feet
    
    hitType = 0;
    
    for (int y = top; y <= bottom; y++) {
        for (int x = left; x <= right; x++) {
            uint8_t t = getTile(x, y);
            if (t > 0) {
                hitType = t;
                hitTx = x;
                hitTy = y;
                if (t == 1 || t == 2) return true; // Solid or spike
            }
        }
    }
    return false;
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

unsigned long lastTime = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    bool fire = input_fire();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(20, 20, "MICRO PLATFORM");
        drawText(20, 40, "PRESS TO START");
        display_render();
        if (fire) {
            resetLevel();
            state = STATE_PLAYING;
        }
    } else if (state == STATE_PLAYING) {
        
        // Input Physics
        if (input_left()) {
            vx -= RUN_ACCEL * dt;
            if (vx < -RUN_SPEED) vx = -RUN_SPEED;
        } else if (input_right()) {
            vx += RUN_ACCEL * dt;
            if (vx > RUN_SPEED) vx = RUN_SPEED;
        } else {
            // Friction
            if (vx > 0) {
                vx -= RUN_FRICTION * dt;
                if (vx < 0) vx = 0;
            } else if (vx < 0) {
                vx += RUN_FRICTION * dt;
                if (vx > 0) vx = 0;
            }
        }
        
        // Jump
        static bool lastJump = false;
        bool curJump = input_jump();
        if (curJump && !lastJump && grounded) {
            vy = JUMP_POWER;
            grounded = false;
            tone(BUZZER_PIN, 400, 30);
        }
        lastJump = curJump;
        
        // Gravity
        vy += GRAVITY * dt;
        if (vy > 300.0f) vy = 300.0f; // Terminal velocity
        
        int hitType, hitTx, hitTy;
        
        // Move X
        px += vx * dt;
        if (checkCollision(px, py, hitType, hitTx, hitTy)) {
            if (hitType == 2) {
                state = STATE_GAMEOVER; tone(BUZZER_PIN, 100, 800);
            } else if (hitType == 1) {
                if (vx > 0) {
                    px = hitTx * TILE_SIZE - 7;
                } else if (vx < 0) {
                    px = (hitTx + 1) * TILE_SIZE;
                }
                vx = 0;
            }
        }
        
        // Move Y
        py += vy * dt;
        grounded = false;
        if (checkCollision(px, py, hitType, hitTx, hitTy)) {
            if (hitType == 2) {
                state = STATE_GAMEOVER; tone(BUZZER_PIN, 100, 800);
            } else if (hitType == 1) {
                if (vy > 0) {
                    py = hitTy * TILE_SIZE - 8;
                    grounded = true;
                } else if (vy < 0) {
                    py = (hitTy + 1) * TILE_SIZE;
                }
                vy = 0;
            }
        }
        
        // Non-solid collisions
        checkCollision(px, py, hitType, hitTx, hitTy); // Re-check to grab items without pushing out
        if (hitType == 3) {
            setTile(hitTx, hitTy, 0);
            coins++;
            tone(BUZZER_PIN, 1200, 50);
        } else if (hitType == 4) {
            state = STATE_WIN;
            tone(BUZZER_PIN, 1500, 400);
        }
        
        // Death by falling off screen
        if (py > SCREEN_H) {
            state = STATE_GAMEOVER;
            tone(BUZZER_PIN, 100, 800);
        }
        
        // Camera follow
        cameraX = px - (SCREEN_W / 2);
        if (cameraX < 0) cameraX = 0;
        int maxCamX = (LEVEL_W * TILE_SIZE) - SCREEN_W;
        if (cameraX > maxCamX) cameraX = maxCamX;
        
        // Render
        display_clear();
        
        int startCol = cameraX / TILE_SIZE;
        int endCol = (cameraX + SCREEN_W) / TILE_SIZE + 1;
        if (endCol > LEVEL_W) endCol = LEVEL_W;
        
        for (int y = 0; y < LEVEL_H; y++) {
            for (int x = startCol; x < endCol; x++) {
                uint8_t t = currentLevel[y * LEVEL_W + x];
                int screenX = (x * TILE_SIZE) - cameraX;
                int screenY = y * TILE_SIZE;
                
                if (t == 1) { // Wall
                    drawRect(screenX, screenY, TILE_SIZE, TILE_SIZE, 1);
                    drawLine(screenX, screenY, screenX+TILE_SIZE, screenY+TILE_SIZE, 1);
                } else if (t == 2) { // Spike
                    drawLine(screenX, screenY+7, screenX+3, screenY+2, 1);
                    drawLine(screenX+3, screenY+2, screenX+7, screenY+7, 1);
                } else if (t == 3) { // Coin
                    drawRect(screenX+2, screenY+2, 4, 4, 1);
                } else if (t == 4) { // Goal flag
                    drawLine(screenX+2, screenY, screenX+2, screenY+8, 1);
                    fillRect(screenX+2, screenY, 6, 4, 1);
                }
            }
        }
        
        // Draw Player
        fillRect((int)px - cameraX + 1, (int)py + 1, 6, 7, 1);
        
        // HUD
        drawText(2, 2, "COINS:");
        drawText(40, 2, coins);
        
        display_render();
        
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(35, 30, "YOU DIED");
        display_render();
        if (fire) state = STATE_INTRO;
    } else if (state == STATE_WIN) {
        display_clear();
        drawText(30, 20, "STAGE CLEAR");
        drawText(30, 40, "COINS:");
        drawText(70, 40, coins);
        display_render();
        if (fire) state = STATE_INTRO;
    }
}

}
