#include "breakout_main.h"
#include "breakout_display.h"
#include "breakout_input.h"

#define BUZZER_PIN 14

namespace Breakout {

const int SCREEN_W = 64;
const int SCREEN_H = 128;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_DEATH,
    STATE_GAMEOVER,
    STATE_WIN
};

GameState state = STATE_INTRO;

struct Paddle {
    float x;
    const float y = 115.0f;
    const float w = 16.0f;
    const float h = 3.0f;
} paddle;

struct Ball {
    float x, y;
    float vx, vy;
    const float size = 2.0f;
} ball;

const int BRICK_COLS = 4;
const int BRICK_ROWS = 5;
const float BRICK_W = 14.0f;
const float BRICK_H = 6.0f;

struct Brick {
    float x, y;
    bool active;
};

Brick bricks[BRICK_COLS * BRICK_ROWS];

int score = 0;
int lives = 3;
int activeBricks = 0;
unsigned long stateTimer = 0;
unsigned long lastFrameTime = 0;

void resetLevel() {
    paddle.x = (SCREEN_W - paddle.w) / 2.0f;
    
    ball.x = paddle.x + paddle.w / 2.0f - ball.size / 2.0f;
    ball.y = paddle.y - ball.size - 1.0f;
    
    // Initial random angle upwards
    float angle = random(210, 330) * PI / 180.0f;
    float speed = 50.0f;
    ball.vx = cos(angle) * speed;
    ball.vy = sin(angle) * speed;
}

void initBricks() {
    float startX = (SCREEN_W - (BRICK_COLS * BRICK_W)) / 2.0f;
    float startY = 20.0f;
    
    activeBricks = 0;
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            int idx = row * BRICK_COLS + col;
            bricks[idx].x = startX + col * BRICK_W;
            bricks[idx].y = startY + row * BRICK_H;
            bricks[idx].active = true;
            activeBricks++;
        }
    }
}

void setup() {
    display_setup();
    input_setup();
    score = 0;
    lives = 3;
    state = STATE_INTRO;
}

bool checkAABB(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    return x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2;
}

void loopPlaying(float dt) {
    // 1. Update Paddle
    float paddleSpeedScale = 120.0f; // px per sec
    float input_x = input_paddle_speed(); // -1.0 to 1.0
    paddle.x += input_x * paddleSpeedScale * dt;
    
    if (paddle.x < 0) paddle.x = 0;
    if (paddle.x > SCREEN_W - paddle.w) paddle.x = SCREEN_W - paddle.w;

    // 2. Update Ball
    ball.x += ball.vx * dt;
    ball.y += ball.vy * dt;

    // 3. Wall Collisions
    if (ball.x <= 0) {
        ball.x = 0;
        ball.vx = abs(ball.vx);
        tone(BUZZER_PIN, 400, 20);
    } else if (ball.x >= SCREEN_W - ball.size) {
        ball.x = SCREEN_W - ball.size;
        ball.vx = -abs(ball.vx);
        tone(BUZZER_PIN, 400, 20);
    }
    
    if (ball.y <= 10) { // Top wall (below HUD)
        ball.y = 10;
        ball.vy = -ball.vy;
        tone(BUZZER_PIN, 400, 20);
    } else if (ball.y >= SCREEN_H) {
        // Death
        state = STATE_DEATH;
        stateTimer = millis();
        tone(BUZZER_PIN, 100, 500);
        return;
    }

    // 4. Paddle Collision
    if (ball.vy > 0 && checkAABB(ball.x, ball.y, ball.size, ball.size, paddle.x, paddle.y, paddle.w, paddle.h)) {
        ball.y = paddle.y - ball.size;
        
        // Calculate bounce angle based on hit position
        float hitPoint = (ball.x + ball.size / 2.0f) - paddle.x;
        float ratio = hitPoint / paddle.w; // 0.0 (left edge) to 1.0 (right edge)
        
        // Map ratio to angle between 210 deg (left) and 330 deg (right)
        float angle = (210.0f + ratio * 120.0f) * PI / 180.0f;
        
        // Increase speed slightly over time
        float currentSpeed = sqrt(ball.vx * ball.vx + ball.vy * ball.vy);
        if (currentSpeed < 120.0f) currentSpeed += 2.0f; 
        
        ball.vx = cos(angle) * currentSpeed;
        ball.vy = -abs(sin(angle) * currentSpeed); // Ensure it goes up
        
        tone(BUZZER_PIN, 600, 30);
    }

    // 5. Brick Collision
    for (int i = 0; i < BRICK_COLS * BRICK_ROWS; i++) {
        if (bricks[i].active) {
            if (checkAABB(ball.x, ball.y, ball.size, ball.size, bricks[i].x, bricks[i].y, BRICK_W - 1, BRICK_H - 1)) {
                bricks[i].active = false;
                score += 10;
                activeBricks--;
                ball.vy = -ball.vy; // Simple vertical bounce
                tone(BUZZER_PIN, 1200, 40);
                
                if (activeBricks <= 0) {
                    state = STATE_WIN;
                    stateTimer = millis();
                    tone(BUZZER_PIN, 1500, 500);
                }
                break; // Only hit one brick per frame
            }
        }
    }

    // Drawing
    display_clear();

    // HUD
    fillRect(0, 0, 64, 9, 0);
    drawText(2, 1, "S:"); drawText(14, 1, score);
    drawText(40, 1, "L:"); drawText(52, 1, lives);
    drawLine(0, 9, 64, 9, 1);

    // Paddle
    fillRect((int)paddle.x, (int)paddle.y, (int)paddle.w, (int)paddle.h, 1);

    // Ball
    fillRect((int)ball.x, (int)ball.y, (int)ball.size, (int)ball.size, 1);

    // Bricks
    for (int i = 0; i < BRICK_COLS * BRICK_ROWS; i++) {
        if (bricks[i].active) {
            drawRect((int)bricks[i].x, (int)bricks[i].y, (int)BRICK_W - 1, (int)BRICK_H - 1, 1);
        }
    }

    display_render();
}

void loop() {
    unsigned long now = millis();
    float dt = (now - lastFrameTime) / 1000.0f;
    lastFrameTime = now;
    if (dt > 0.1f) dt = 0.1f;

    bool fire_pressed = input_fire();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(8, 20, "BREAKOUT");
        drawText(8, 60, "PRESS FIRE");
        display_render();

        if (fire_pressed) {
            score = 0;
            lives = 3;
            initBricks();
            resetLevel();
            state = STATE_PLAYING;
            lastFrameTime = millis();
        }
    } else if (state == STATE_PLAYING) {
        loopPlaying(dt);
    } else if (state == STATE_DEATH) {
        display_clear();
        drawText(18, 40, "OOPS!");
        display_render();

        if (millis() - stateTimer > 1500) {
            lives--;
            if (lives <= 0) {
                state = STATE_GAMEOVER;
                tone(BUZZER_PIN, 100, 600);
            } else {
                resetLevel();
                state = STATE_PLAYING;
                lastFrameTime = millis();
            }
        }
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(5, 30, "GAME OVER");
        drawText(2, 60, "SCORE:");
        drawText(40, 60, score);
        display_render();

        if (fire_pressed) {
            state = STATE_INTRO;
        }
    } else if (state == STATE_WIN) {
        display_clear();
        drawText(15, 30, "YOU WIN!");
        drawText(2, 60, "SCORE:");
        drawText(40, 60, score);
        display_render();

        if (fire_pressed) {
            state = STATE_INTRO;
        }
    }
}

}
