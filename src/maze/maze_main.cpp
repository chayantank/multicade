#include "maze_main.h"
#include "maze_display.h"
#include "maze_input.h"

#define BUZZER_PIN 14

namespace Maze {

const int SCREEN_W = 128;
const int SCREEN_H = 64;

const int MAX_COLS = 63;
const int MAX_ROWS = 31;

uint8_t grid[MAX_COLS][MAX_ROWS];

int px = 1;
int py = 1;
int ex = 1;
int ey = 1;

int level = 1;
int cols = 15;
int rows = 7;
int tileSize = 8;
int offsetX = 4;
int offsetY = 4;

unsigned long lastMoveTime = 0;
unsigned long stateTimer = 0;

enum GameState {
    STATE_INTRO,
    STATE_GENERATING,
    STATE_PLAYING,
    STATE_LEVEL_CLEAR,
    STATE_WIN
};

GameState state = STATE_INTRO;

void carve(int cx, int cy) {
    grid[cx][cy] = 1;
    
    int dirs[4] = {0, 1, 2, 3};
    for (int i = 0; i < 4; i++) {
        int r = random(0, 4);
        int t = dirs[i]; dirs[i] = dirs[r]; dirs[r] = t;
    }
    
    for (int i = 0; i < 4; i++) {
        int nx = cx;
        int ny = cy;
        
        if (dirs[i] == 0) ny -= 2;
        else if (dirs[i] == 1) nx += 2;
        else if (dirs[i] == 2) ny += 2;
        else if (dirs[i] == 3) nx -= 2;
        
        if (nx > 0 && nx < cols - 1 && ny > 0 && ny < rows - 1 && grid[nx][ny] == 0) {
            grid[cx + (nx - cx) / 2][cy + (ny - cy) / 2] = 1;
            carve(nx, ny);
        }
    }
}

void generateMaze() {
    for (int x = 0; x < cols; x++) {
        for (int y = 0; y < rows; y++) {
            grid[x][y] = 0; // Everything is wall
        }
    }
    
    carve(1, 1);
    
    ex = cols - 2;
    ey = rows - 2;
    grid[ex][ey] = 2; // Set exit
    px = 1;
    py = 1;
}

void setupLevel() {
    if (level == 1) {
        cols = 15; rows = 7; tileSize = 8;
    } else if (level == 2) {
        cols = 31; rows = 15; tileSize = 4;
    } else {
        cols = 63; rows = 31; tileSize = 2;
    }
    
    // For higher levels, remove some dead ends to create loops (Braid Maze)
    // This makes it much more misleading!
    if (level > 3) {
        int deadEndRemovals = (level - 3) * 50; // More loops on higher levels
        for(int i=0; i<deadEndRemovals; i++) {
            int rx = random(1, cols-1);
            int ry = random(1, rows-1);
            if (grid[rx][ry] == 0) { // If it's a wall
                // Check if removing it connects two paths
                int paths = 0;
                if (grid[rx-1][ry] != 0) paths++;
                if (grid[rx+1][ry] != 0) paths++;
                if (grid[rx][ry-1] != 0) paths++;
                if (grid[rx][ry+1] != 0) paths++;
                if (paths == 2) {
                    grid[rx][ry] = 1; // Carve loop!
                }
            }
        }
    }
    
    offsetX = (SCREEN_W - (cols * tileSize)) / 2;
    offsetY = (SCREEN_H - (rows * tileSize)) / 2;
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

void loop() {
    bool fire = input_fire();
    unsigned long now = millis();
    
    if (state == STATE_INTRO) {
        display_clear();
        drawText(30, 20, "MAZE RUNNER");
        drawText(20, 40, "PRESS TO START");
        display_render();
        
        if (fire) {
            level = 1;
            setupLevel();
            state = STATE_GENERATING;
        }
    } else if (state == STATE_GENERATING) {
        display_clear();
        drawText(25, 25, "GENERATING...");
        display_render();
        
        generateMaze();
        state = STATE_PLAYING;
    } else if (state == STATE_PLAYING) {
        if (now - lastMoveTime > 100) {
            int nx = px;
            int ny = py;
            bool moved = false;
            
            if (input_up()) { ny--; moved = true; }
            else if (input_down()) { ny++; moved = true; }
            else if (input_left()) { nx--; moved = true; }
            else if (input_right()) { nx++; moved = true; }
            
            if (moved) {
                if (grid[nx][ny] != 0) { // Not a wall
                    px = nx;
                    py = ny;
                    tone(BUZZER_PIN, 800, 10);
                }
                lastMoveTime = now;
            }
        }
        
        // Check win
        if (px == ex && py == ey) {
            if (level < 10) {
                state = STATE_LEVEL_CLEAR;
                stateTimer = now;
                tone(BUZZER_PIN, 1200, 300);
            } else {
                state = STATE_WIN;
                stateTimer = now;
                tone(BUZZER_PIN, 1500, 500);
            }
        }
        
        // Draw
        display_clear();
        
        for (int x = 0; x < cols; x++) {
            for (int y = 0; y < rows; y++) {
                if (grid[x][y] == 0) {
                    fillRect(offsetX + x * tileSize, offsetY + y * tileSize, tileSize, tileSize, 1);
                } else if (grid[x][y] == 2) {
                    // Draw exit as blinking
                    if ((now / 250) % 2 == 0) {
                        fillRect(offsetX + x * tileSize, offsetY + y * tileSize, tileSize, tileSize, 1);
                    } else {
                        drawRect(offsetX + x * tileSize, offsetY + y * tileSize, tileSize, tileSize, 1);
                    }
                }
            }
        }
        
        // Draw player
        if (tileSize >= 4) {
            fillRect(offsetX + px * tileSize + 1, offsetY + py * tileSize + 1, tileSize - 2, tileSize - 2, 1);
        } else {
            // For tileSize 2, just draw the whole block
            fillRect(offsetX + px * tileSize, offsetY + py * tileSize, tileSize, tileSize, 1);
        }
        
        // HUD
        drawText(2, 2, "L:");
        drawText(15, 2, level);
        
        display_render();
        
    } else if (state == STATE_LEVEL_CLEAR) {
        display_clear();
        drawText(20, 25, "LEVEL COMPLETE!");
        display_render();
        
        if (fire && now - stateTimer > 1000) {
            level++;
            setupLevel();
            state = STATE_GENERATING;
        }
    } else if (state == STATE_WIN) {
        display_clear();
        drawText(20, 25, "YOU ESCAPED!");
        display_render();
        
        if (fire && now - stateTimer > 1000) {
            state = STATE_INTRO;
        }
    }
}

}
