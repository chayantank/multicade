#include "hopper_main.h"
#include "hopper_display.h"
#include "hopper_input.h"
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 menuDisplay;

#define BUZZER_PIN 14

namespace Hopper {

const int SCREEN_W = 64;
const int SCREEN_H = 128;
const int TILE = 8;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_WIN
};

GameState state = STATE_INTRO;
unsigned long stateTimer = 0;

int px = 4; // Grid coords (0-7)
int py = 15; // Grid coords (0-15)
float rx = 4.0f; // Real X for smooth log riding

int score = 0;
int lives = 3;
int level = 1;
int highestRow = 15;

unsigned long lastMoveTime = 0;

struct Obstacle {
    float x;
    int y; // Row
    int w;
    float speed;
    int type; // 1=Car, 2=Log
};

const int MAX_OBSTACLES = 30;
Obstacle obs[MAX_OBSTACLES];

void initLevel() {
    px = 3;
    py = 15;
    rx = px * TILE;
    
    float baseSpeed = 15.0f + (level * 5.0f);
    
    // Clear
    for(int i=0; i<MAX_OBSTACLES; i++) {
        obs[i].type = 0;
    }
    
    int idx = 0;
    
    // Rows 10-14: Cars
    for(int r=10; r<=14; r++) {
        float speed = baseSpeed + random(-5, 15);
        int dir = (r % 2 == 0) ? 1 : -1;
        speed *= dir;
        
        int count = random(1, 3);
        float gap = 64.0f / count;
        for(int i=0; i<count && idx<MAX_OBSTACLES; i++) {
            obs[idx].y = r;
            obs[idx].x = i * gap;
            obs[idx].w = random(1, 3) * TILE;
            obs[idx].speed = speed;
            obs[idx].type = 1; // Car
            idx++;
        }
    }
    
    // Rows 2-8: Logs
    for(int r=2; r<=8; r++) {
        float speed = baseSpeed * 0.8f + random(-5, 10);
        int dir = (r % 2 == 1) ? 1 : -1;
        speed *= dir;
        
        int count = random(2, 4);
        float gap = 64.0f / count;
        for(int i=0; i<count && idx<MAX_OBSTACLES; i++) {
            obs[idx].y = r;
            obs[idx].x = i * gap;
            obs[idx].w = random(2, 4) * TILE;
            obs[idx].speed = speed;
            obs[idx].type = 2; // Log
            idx++;
        }
    }
}

void resetGame() {
    score = 0;
    lives = 3;
    level = 1;
    highestRow = 15;
    initLevel();
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
    
    bool action = input_action();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(15, 40, "HOPPER");
        drawText(5, 70, "PRESS TO");
        drawText(15, 80, "START");
        display_render();
        
        if (action) {
            resetGame();
            state = STATE_PLAYING;
        }
    } else if (state == STATE_PLAYING) {
        
        // Input
        if (now - lastMoveTime > 150) {
            int dir = input_get_direction();
            bool moved = false;
            
            if (dir == 0 && py > 0) { 
                py--; 
                moved = true; 
                if (py < highestRow) {
                    score += 10;
                    highestRow = py;
                }
            }
            else if (dir == 1 && px < 7) { px++; moved = true; }
            else if (dir == 2 && py < 15) { py++; moved = true; }
            else if (dir == 3 && px > 0) { px--; moved = true; }
            
            if (moved) {
                rx = px * TILE;
                lastMoveTime = now;
                tone(BUZZER_PIN, 800, 20);
            }
        }
        
        // Update Obstacles
        for(int i=0; i<MAX_OBSTACLES; i++) {
            if (obs[i].type > 0) {
                obs[i].x += obs[i].speed * dt;
                
                // Wrap
                if (obs[i].speed > 0 && obs[i].x > SCREEN_W) obs[i].x = -obs[i].w;
                else if (obs[i].speed < 0 && obs[i].x < -obs[i].w) obs[i].x = SCREEN_W;
            }
        }
        
        bool dead = false;
        
        // Check Collisions
        bool onLog = false;
        
        if (py >= 10 && py <= 14) {
            // Road
            for(int i=0; i<MAX_OBSTACLES; i++) {
                if (obs[i].type == 1 && obs[i].y == py) {
                    if (rx + 2 < obs[i].x + obs[i].w && rx + TILE - 2 > obs[i].x) {
                        dead = true;
                    }
                }
            }
        } else if (py >= 2 && py <= 8) {
            // River
            for(int i=0; i<MAX_OBSTACLES; i++) {
                if (obs[i].type == 2 && obs[i].y == py) {
                    if (rx + 2 < obs[i].x + obs[i].w && rx + TILE - 2 > obs[i].x) {
                        onLog = true;
                        rx += obs[i].speed * dt;
                        px = (int)(rx + 4) / TILE; // Update logical px
                        break;
                    }
                }
            }
            if (!onLog) dead = true; // Drowned
        }
        
        // Off screen check
        if (rx < -TILE || rx > SCREEN_W) dead = true;
        
        if (dead) {
            lives--;
            if (lives <= 0) {
                state = STATE_GAMEOVER;
                stateTimer = now;
                tone(BUZZER_PIN, 100, 1000);
            } else {
                px = 3; py = 15; rx = px * TILE;
                tone(BUZZER_PIN, 200, 500);
            }
        }
        
        // Win
        if (py == 0) {
            level++;
            score += 1000;
            highestRow = 15;
            tone(BUZZER_PIN, 1500, 500);
            initLevel();
        }
        
        // Draw
        display_clear();
        
        // River background
        fillRect(0, 2*TILE, SCREEN_W, 7*TILE, 1);
        for(int i=0; i<SCREEN_W; i+=4) {
            int yOffset = (millis()/200 % 2);
            for(int y=2*TILE; y<9*TILE; y+=4) {
                drawPixel(i, y + yOffset, 0); // Ripple
            }
        }
        
        // Safe zones
        drawLine(0, 1*TILE, SCREEN_W, 1*TILE, 1);
        drawLine(0, 9*TILE, SCREEN_W, 9*TILE, 1);
        drawLine(0, 15*TILE, SCREEN_W, 15*TILE, 1);
        
        // Obstacles
        for(int i=0; i<MAX_OBSTACLES; i++) {
            if (obs[i].type == 1) {
                // Car
                int cx = (int)obs[i].x;
                int cy = obs[i].y * TILE;
                drawRect(cx, cy+1, obs[i].w, 6, 1);
                fillRect(cx+2, cy+2, obs[i].w-4, 4, 1);
            } else if (obs[i].type == 2) {
                // Log (inverted because it's on white river)
                int lx = (int)obs[i].x;
                int ly = obs[i].y * TILE;
                fillRect(lx, ly+1, obs[i].w, 6, 0);
                drawLine(lx+2, ly+3, lx+obs[i].w-2, ly+3, 1);
            }
        }
        
        // Player
        int pDrawX = (int)rx;
        int pDrawY = py * TILE;
        int pColor = (py >= 2 && py <= 8) ? 0 : 1; // Invert on river
        
        // Frog shape
        drawRect(pDrawX+1, pDrawY+2, 6, 4, pColor);
        drawPixel(pDrawX, pDrawY+1, pColor); drawPixel(pDrawX, pDrawY+6, pColor);
        drawPixel(pDrawX+7, pDrawY+1, pColor); drawPixel(pDrawX+7, pDrawY+6, pColor);
        
        // HUD
        fillRect(0, 0, SCREEN_W, 8, 1);
        menuDisplay.setTextColor(BLACK, WHITE);
        menuDisplay.setTextSize(1);
        menuDisplay.setCursor(1, 0);
        menuDisplay.print(score);
        
        for(int i=0; i<lives; i++) {
            drawPixel(SCREEN_W - 3 - (i*4), 3, 0);
            drawPixel(SCREEN_W - 4 - (i*4), 4, 0);
            drawPixel(SCREEN_W - 2 - (i*4), 4, 0);
            drawPixel(SCREEN_W - 3 - (i*4), 5, 0);
        }
        
        display_render();
        
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(5, 40, "GAME OVER");
        drawText(5, 60, "SCORE:");
        drawText(5, 75, score);
        display_render();
        
        if (action && now - stateTimer > 1000) {
            state = STATE_INTRO;
        }
    }
}

}
