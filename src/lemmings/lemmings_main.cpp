#include "lemmings_main.h"
#include "lemmings_display.h"
#include "lemmings_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Lemmings {

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;

enum Job {
    WALKER = 0,
    BLOCKER = 1,
    BUILDER = 2,
    DIGGER = 3,
    FALLER = 4
};

struct Lem {
    float x, y;
    int dir;
    Job job;
    bool active;
    bool dead;
    bool saved;
    int buildCount;
};

const int MAX_LEMS = 10;
Lem lems[MAX_LEMS];

uint8_t grid[128][54];

float cursorX = 64;
float cursorY = 27;

int selectedTool = 1;
int toolsLeft[4] = {0, 3, 5, 5};

int savedCount = 0;
int spawnCount = 0;
unsigned long lastSpawnTime = 0;
float timeScale = 1.0f; // Added speed up

void initGrid() {
    for(int x=0; x<128; x++) {
        for(int y=0; y<54; y++) {
            grid[x][y] = 0;
            if (y > 45) grid[x][y] = 1;
            if (x > 50 && x < 60 && y > 20 && y <= 45) grid[x][y] = 1;
            if (x > 80 && x < 100 && y > 45) grid[x][y] = 0;
            if (x > 10 && x < 40 && y > 30 && y < 35) grid[x][y] = 1;
        }
    }
}

void resetGame() {
    initGrid();
    for(int i=0; i<MAX_LEMS; i++) {
        lems[i].active = false;
        lems[i].dead = false;
        lems[i].saved = false;
    }
    savedCount = 0;
    spawnCount = 0;
    timeScale = 1.0f;
    cursorX = 64; cursorY = 27;
    toolsLeft[1] = 3; toolsLeft[2] = 5; toolsLeft[3] = 5;
    state = STATE_PLAYING;
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

unsigned long lastTime = 0;
unsigned long stateTimer = 0;

void loop() {
    unsigned long now = millis();
    float rawDt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (rawDt > 0.1f) rawDt = 0.1f;
    
    if (state == STATE_INTRO) {
        display_clear();
        drawRect(0, 0, 128, 64, 1);
        drawRect(2, 2, 124, 60, 1);
        drawText(40, 20, "LEMMINGS");
        
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
    
    if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(35, 20, "GAME OVER");
        drawText(30, 30, "SAVED:");
        drawText(70, 30, savedCount);
        drawText(15, 50, "CLICK TO RESTART");
        display_render();
        if (input_action() && now - stateTimer > 500) {
            tone(BUZZER_PIN, 800, 100); delay(200);
            resetGame();
        }
        return;
    }
    
    float dt = rawDt * timeScale; // Apply speedup
    
    if (spawnCount < MAX_LEMS && now - lastSpawnTime > 2000 / timeScale) {
        lems[spawnCount].active = true;
        lems[spawnCount].x = 20;
        lems[spawnCount].y = 10;
        lems[spawnCount].dir = 1;
        lems[spawnCount].job = FALLER;
        spawnCount++;
        lastSpawnTime = now;
    }
    
    int jx = input_x() - 2048;
    int jy = input_y() - 2048;
    
    // Cursor moves normally regardless of time scale
    if (abs(jx) > 500) cursorX += (jx > 0 ? 1 : -1) * 80.0f * rawDt;
    if (abs(jy) > 500) cursorY += (jy > 0 ? 1 : -1) * 80.0f * rawDt;
    
    if (cursorX < 0) cursorX = 0; if (cursorX > 127) cursorX = 127;
    if (cursorY < 0) cursorY = 0; if (cursorY > 63) cursorY = 63;
    
    if (input_action()) {
        if (cursorY > 54) {
            if (cursorX < 30) selectedTool = 1;
            else if (cursorX < 60) selectedTool = 2;
            else if (cursorX < 90) selectedTool = 3;
            else {
                // Clicked the Fast Forward button
                timeScale = (timeScale > 1.5f) ? 1.0f : 2.5f;
                tone(BUZZER_PIN, 2000, 50);
            }
            if (cursorX < 90) tone(BUZZER_PIN, 1500, 30);
        } else {
            for(int i=0; i<MAX_LEMS; i++) {
                if (lems[i].active && !lems[i].dead && !lems[i].saved) {
                    if (abs(lems[i].x - cursorX) < 4 && abs(lems[i].y - cursorY) < 4) {
                        if (toolsLeft[selectedTool] > 0) {
                            lems[i].job = (Job)selectedTool;
                            lems[i].buildCount = 0;
                            toolsLeft[selectedTool]--;
                            tone(BUZZER_PIN, 800, 50);
                            break;
                        }
                    }
                }
            }
        }
    }
    
    int activeLems = 0;
    for(int i=0; i<MAX_LEMS; i++) {
        if (lems[i].active && !lems[i].dead && !lems[i].saved) {
            activeLems++;
            
            int ix = (int)lems[i].x;
            int iy = (int)lems[i].y;
            
            if (iy < 53 && grid[ix][iy+1] == 0) {
                lems[i].y += 30.0f * dt;
                lems[i].job = FALLER;
            } else {
                if (lems[i].job == FALLER) lems[i].job = WALKER;
                
                if (lems[i].job == WALKER) {
                    lems[i].x += lems[i].dir * 15.0f * dt;
                    int nx = (int)lems[i].x;
                    if (nx >= 0 && nx < 128 && grid[nx][iy] == 1) {
                        lems[i].dir *= -1;
                        lems[i].x += lems[i].dir * 2;
                    }
                    for(int j=0; j<MAX_LEMS; j++) {
                        if (i!=j && lems[j].active && lems[j].job == BLOCKER) {
                            if (abs(lems[i].x - lems[j].x) < 2 && abs(lems[i].y - lems[j].y) < 2) {
                                lems[i].dir *= -1;
                                lems[i].x += lems[i].dir * 2;
                            }
                        }
                    }
                } else if (lems[i].job == DIGGER) {
                    lems[i].y += 10.0f * dt;
                    if (iy < 53) grid[ix][iy+1] = 0;
                    if (iy < 53 && grid[ix][iy+2] == 0) lems[i].job = FALLER;
                } else if (lems[i].job == BUILDER) {
                    if (lems[i].buildCount < 10) {
                        // Use a timed accumulator rather than now%500 to respect time scale
                        static float buildAcc = 0;
                        buildAcc += dt;
                        if (buildAcc > 0.5f) {
                            buildAcc = 0;
                            int sx = ix + lems[i].dir;
                            if (sx>=0 && sx<128 && iy>0) {
                                grid[sx][iy] = 1;
                                lems[i].x += lems[i].dir;
                                lems[i].y -= 1;
                                lems[i].buildCount++;
                            }
                        }
                    } else {
                        lems[i].job = WALKER;
                    }
                }
            }
            
            if (lems[i].x < 0 || lems[i].x > 127 || lems[i].y > 54) {
                lems[i].dead = true;
                tone(BUZZER_PIN, 100, 100);
            }
            
            if (abs(lems[i].x - 110) < 4 && abs(lems[i].y - 45) < 4) {
                lems[i].saved = true;
                savedCount++;
                tone(BUZZER_PIN, 1500, 100);
            }
        }
    }
    
    if (spawnCount == MAX_LEMS && activeLems == 0) {
        state = STATE_GAMEOVER;
        stateTimer = now;
    }
    
    display_clear();
    
    for(int x=0; x<128; x++) {
        for(int y=0; y<54; y++) {
            if (grid[x][y] == 1) drawPixel(x, y, 1);
        }
    }
    
    drawRect(108, 41, 6, 5, 1);
    
    for(int i=0; i<MAX_LEMS; i++) {
        if (lems[i].active && !lems[i].dead && !lems[i].saved) {
            int lx = (int)lems[i].x;
            int ly = (int)lems[i].y;
            drawPixel(lx, ly, 1);
            drawPixel(lx, ly-1, 1);
            if (lems[i].job == BLOCKER) {
                drawPixel(lx-1, ly, 1);
                drawPixel(lx+1, ly, 1);
            }
        }
    }
    
    drawLine(0, 54, 128, 54, 1);
    drawText(2, 56, "B"); drawText(10, 56, toolsLeft[1]);
    drawText(32, 56, "S"); drawText(40, 56, toolsLeft[2]);
    drawText(62, 56, "D"); drawText(70, 56, toolsLeft[3]);
    
    // Fast forward button text
    if (timeScale > 1.5f) drawText(100, 56, ">>>");
    else drawText(100, 56, ">>");
    
    if (selectedTool == 1) drawRect(0, 55, 25, 9, 1);
    else if (selectedTool == 2) drawRect(30, 55, 25, 9, 1);
    else if (selectedTool == 3) drawRect(60, 55, 25, 9, 1);
    
    drawLine((int)cursorX-2, (int)cursorY, (int)cursorX+2, (int)cursorY, 1);
    drawLine((int)cursorX, (int)cursorY-2, (int)cursorX, (int)cursorY+2, 1);
    
    display_render();
}

}
