#include "snake_main.h"
#include "snake_display.h"
#include "snake_input.h"

#define BUZZER_PIN 14

namespace Snake {

const int SCREEN_W = 128;
const int SCREEN_H = 64;
const int TILE_SIZE = 4;
const int COLS = SCREEN_W / TILE_SIZE; // 32
const int ROWS = SCREEN_H / TILE_SIZE; // 16

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_DEATH,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;

struct Node {
    int x, y;
};

const int MAX_LEN = 200;
Node snake[MAX_LEN];
int snakeLen = 3;
int dir = 1; // 0=UP, 1=RIGHT, 2=DOWN, 3=LEFT
int nextDir = 1;

int appleX = 0;
int appleY = 0;

int score = 0;
unsigned long stateTimer = 0;
unsigned long lastMoveTime = 0;

// Input Queue
int inputQueue[2] = {-1, -1};
void pushInput(int d) {
    if (inputQueue[0] == -1) inputQueue[0] = d;
    else if (inputQueue[1] == -1) inputQueue[1] = d;
}
int popInput() {
    int d = inputQueue[0];
    inputQueue[0] = inputQueue[1];
    inputQueue[1] = -1;
    return d;
}

bool lastUp = false, lastRight = false, lastDown = false, lastLeft = false;

void spawnApple() {
    bool valid = false;
    while (!valid) {
        appleX = random(0, COLS);
        appleY = random(0, ROWS);
        valid = true;
        for (int i = 0; i < snakeLen; i++) {
            if (snake[i].x == appleX && snake[i].y == appleY) {
                valid = false;
                break;
            }
        }
    }
}

void resetLevel() {
    snakeLen = 3;
    dir = 1;
    nextDir = 1;
    score = 0;
    
    snake[0] = {10, 8};
    snake[1] = {9, 8};
    snake[2] = {8, 8};
    
    inputQueue[0] = -1;
    inputQueue[1] = -1;
    lastUp = input_up();
    lastRight = input_right();
    lastDown = input_down();
    lastLeft = input_left();
    
    spawnApple();
    lastMoveTime = millis();
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

void loopPlaying() {
    // Edge detection for input
    bool curUp = input_up();
    bool curRight = input_right();
    bool curDown = input_down();
    bool curLeft = input_left();
    
    if (curUp && !lastUp) pushInput(0);
    if (curRight && !lastRight) pushInput(1);
    if (curDown && !lastDown) pushInput(2);
    if (curLeft && !lastLeft) pushInput(3);
    
    lastUp = curUp;
    lastRight = curRight;
    lastDown = curDown;
    lastLeft = curLeft;

    // Movement Tick
    int moveDelay = 150 - (snakeLen * 2);
    if (moveDelay < 40) moveDelay = 40;
    
    if (millis() - lastMoveTime > moveDelay) {
        int d = popInput();
        if (d != -1) {
            if ((d == 0 && dir != 2) || (d == 1 && dir != 3) || 
                (d == 2 && dir != 0) || (d == 3 && dir != 1)) {
                dir = d;
            }
        }
        
        // Calculate new head position
        int headX = snake[0].x;
        int headY = snake[0].y;
        
        if (dir == 0) headY--;
        else if (dir == 1) headX++;
        else if (dir == 2) headY++;
        else if (dir == 3) headX--;
        
        // Collision with walls
        if (headX < 0 || headX >= COLS || headY < 0 || headY >= ROWS) {
            state = STATE_DEATH;
            stateTimer = millis();
            tone(BUZZER_PIN, 100, 500);
            return;
        }
        
        // Collision with self
        for (int i = 0; i < snakeLen; i++) {
            if (snake[i].x == headX && snake[i].y == headY) {
                state = STATE_DEATH;
                stateTimer = millis();
                tone(BUZZER_PIN, 100, 500);
                return;
            }
        }
        
        // Move body
        for (int i = snakeLen - 1; i > 0; i--) {
            snake[i] = snake[i - 1];
        }
        snake[0] = {headX, headY};
        
        // Apple logic
        if (headX == appleX && headY == appleY) {
            if (snakeLen < MAX_LEN) {
                snake[snakeLen] = snake[snakeLen - 1]; // Duplicate last tail node temporarily
                snakeLen++;
            }
            score += 10;
            tone(BUZZER_PIN, 1500, 50);
            spawnApple();
        } else {
            // Little move tick sound
            // tone(BUZZER_PIN, 400, 5); // Disabled to not be annoying
        }
        
        lastMoveTime = millis();
    }

    // Drawing
    display_clear();
    
    // Draw Apple
    fillRect(appleX * TILE_SIZE, appleY * TILE_SIZE, TILE_SIZE, TILE_SIZE, 1);
    
    // Draw Snake
    for (int i = 0; i < snakeLen; i++) {
        // Draw head slightly differently or just simple rects
        fillRect(snake[i].x * TILE_SIZE, snake[i].y * TILE_SIZE, TILE_SIZE - 1, TILE_SIZE - 1, 1);
    }
    
    // Draw Score (Top right)
    drawText(80, 1, "S:");
    drawText(95, 1, score);
    
    display_render();
}

void loop() {
    bool currentFire = input_fire();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(48, 20, "SNAKE");
        drawText(30, 40, "PRESS FIRE");
        display_render();

        if (currentFire) {
            resetLevel();
            state = STATE_PLAYING;
            lastMoveTime = millis();
        }
    } else if (state == STATE_PLAYING) {
        loopPlaying();
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

        if (currentFire) {
            state = STATE_INTRO;
        }
    }
}

}
