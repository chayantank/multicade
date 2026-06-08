#include "flappy_main.h"
#include "flappy_display.h"
#include "flappy_input.h"

#define BUZZER_PIN 14

namespace Flappy {

const int SCREEN_W = 128;
const int SCREEN_H = 64;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_DEATH,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;

struct Bird {
    const float x = 20.0f;
    float y;
    float vy;
    const int r = 2;
} bird;

const int MAX_PIPES = 3;
struct Pipe {
    float x;
    float gapY;
    bool active;
    bool passed;
};

Pipe pipes[MAX_PIPES];
const float PIPE_W = 10.0f;
float gapSize = 25.0f;
float scrollSpeed = 40.0f;

int score = 0;
unsigned long stateTimer = 0;
unsigned long lastFrameTime = 0;
bool lastFlap = false;

void spawnPipe(int idx, float xOffset) {
    pipes[idx].x = xOffset;
    pipes[idx].gapY = random(10, SCREEN_H - 10 - (int)gapSize);
    pipes[idx].active = true;
    pipes[idx].passed = false;
}

void resetLevel() {
    bird.y = SCREEN_H / 2.0f;
    bird.vy = 0;
    
    score = 0;
    gapSize = 25.0f;
    scrollSpeed = 45.0f;
    
    for (int i = 0; i < MAX_PIPES; i++) {
        spawnPipe(i, SCREEN_W + i * 60.0f);
    }
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

bool checkAABB(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    return x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2;
}

void loopPlaying(float dt) {
    bool currentFlap = input_flap();
    
    if (currentFlap && !lastFlap) {
        bird.vy = -70.0f; // Flap strength
        tone(BUZZER_PIN, 800, 20);
    }
    lastFlap = currentFlap;
    
    // Gravity
    bird.vy += 250.0f * dt;
    bird.y += bird.vy * dt;
    
    // Bounds checking
    if (bird.y - bird.r < 0 || bird.y + bird.r > SCREEN_H) {
        state = STATE_DEATH;
        stateTimer = millis();
        tone(BUZZER_PIN, 100, 500);
        return;
    }
    
    // Pipes
    for (int i = 0; i < MAX_PIPES; i++) {
        if (pipes[i].active) {
            pipes[i].x -= scrollSpeed * dt;
            
            // Collision
            // Top pipe
            if (checkAABB(bird.x - bird.r, bird.y - bird.r, bird.r * 2, bird.r * 2,
                          pipes[i].x, 0, PIPE_W, pipes[i].gapY)) {
                state = STATE_DEATH;
                stateTimer = millis();
                tone(BUZZER_PIN, 100, 500);
                return;
            }
            
            // Bottom pipe
            if (checkAABB(bird.x - bird.r, bird.y - bird.r, bird.r * 2, bird.r * 2,
                          pipes[i].x, pipes[i].gapY + gapSize, PIPE_W, SCREEN_H - (pipes[i].gapY + gapSize))) {
                state = STATE_DEATH;
                stateTimer = millis();
                tone(BUZZER_PIN, 100, 500);
                return;
            }
            
            // Scoring
            if (!pipes[i].passed && pipes[i].x + PIPE_W < bird.x - bird.r) {
                pipes[i].passed = true;
                score++;
                tone(BUZZER_PIN, 1200, 30);
                
                // Increase difficulty
                scrollSpeed += 2.0f;
                if (gapSize > 15.0f) gapSize -= 0.5f;
            }
            
            // Respawn
            if (pipes[i].x + PIPE_W < 0) {
                // Find rightmost pipe
                float maxX = 0;
                for (int j = 0; j < MAX_PIPES; j++) {
                    if (pipes[j].x > maxX) maxX = pipes[j].x;
                }
                spawnPipe(i, maxX + 60.0f);
            }
        }
    }
    
    // Draw
    display_clear();
    
    // Draw Pipes
    for (int i = 0; i < MAX_PIPES; i++) {
        if (pipes[i].active) {
            fillRect((int)pipes[i].x, 0, (int)PIPE_W, (int)pipes[i].gapY, 1);
            fillRect((int)pipes[i].x, (int)(pipes[i].gapY + gapSize), (int)PIPE_W, SCREEN_H - (int)(pipes[i].gapY + gapSize), 1);
        }
    }
    
    // Draw Bird
    drawCircle((int)bird.x, (int)bird.y, bird.r, 1);
    
    // Draw Score
    fillRect(0, 0, 30, 9, 0); // Background for score
    drawText(2, 1, score);
    
    display_render();
}

void loop() {
    unsigned long now = millis();
    float dt = (now - lastFrameTime) / 1000.0f;
    lastFrameTime = now;
    if (dt > 0.1f) dt = 0.1f;

    bool currentFlap = input_flap();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(40, 20, "FLAPPY");
        drawText(30, 40, "PRESS FIRE");
        display_render();

        if (currentFlap && !lastFlap) {
            resetLevel();
            state = STATE_PLAYING;
            lastFrameTime = millis();
        }
    } else if (state == STATE_PLAYING) {
        loopPlaying(dt);
    } else if (state == STATE_DEATH) {
        display_clear();
        drawText(45, 20, "CRASH!");
        display_render();

        if (millis() - stateTimer > 1500) {
            state = STATE_GAMEOVER;
        }
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(35, 20, "GAME OVER");
        drawText(40, 40, "SCORE:");
        drawText(80, 40, score);
        display_render();

        if (currentFlap && !lastFlap) {
            state = STATE_INTRO;
        }
    }
    lastFlap = currentFlap;
}

}
