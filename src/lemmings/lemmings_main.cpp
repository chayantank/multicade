#include "lemmings_main.h"
#include "lemmings_display.h"
#include "lemmings_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Lemmings {

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
bool gameOver = false;

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
    gameOver = false;
    cursorX = 64; cursorY = 27;
    toolsLeft[1] = 3; toolsLeft[2] = 5; toolsLeft[3] = 5;
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
        drawText(30, 30, "SAVED:");
        drawText(70, 30, savedCount);
        display_render();
        if (input_action()) {
            tone(BUZZER_PIN, 800, 100); delay(200);
            resetGame();
        }
        return;
    }
    
    if (spawnCount < MAX_LEMS && now - lastSpawnTime > 2000) {
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
    
    if (abs(jx) > 500) cursorX += (jx > 0 ? 1 : -1) * 80.0f * dt;
    if (abs(jy) > 500) cursorY += (jy > 0 ? 1 : -1) * 80.0f * dt;
    
    if (cursorX < 0) cursorX = 0; if (cursorX > 127) cursorX = 127;
    if (cursorY < 0) cursorY = 0; if (cursorY > 63) cursorY = 63;
    
    if (input_action()) {
        if (cursorY > 54) {
            if (cursorX < 40) selectedTool = 1;
            else if (cursorX < 80) selectedTool = 2;
            else selectedTool = 3;
            tone(BUZZER_PIN, 1500, 30);
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
                        if (now % 500 < 50) {
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
        gameOver = true;
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
    drawText(2, 56, "B:"); drawText(14, 56, toolsLeft[1]);
    drawText(42, 56, "S:"); drawText(54, 56, toolsLeft[2]);
    drawText(82, 56, "D:"); drawText(94, 56, toolsLeft[3]);
    
    if (selectedTool == 1) drawRect(0, 55, 30, 9, 1);
    else if (selectedTool == 2) drawRect(40, 55, 30, 9, 1);
    else drawRect(80, 55, 30, 9, 1);
    
    drawLine((int)cursorX-2, (int)cursorY, (int)cursorX+2, (int)cursorY, 1);
    drawLine((int)cursorX, (int)cursorY-2, (int)cursorX, (int)cursorY+2, 1);
    
    display_render();
}

}
