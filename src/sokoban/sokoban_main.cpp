#include "sokoban_main.h"
#include "sokoban_display.h"
#include "sokoban_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Sokoban {

const int SCREEN_W = 128;
const int SCREEN_H = 64;
const int TILE = 8;
const int COLS = 16;
const int ROWS = 8;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_LEVEL_CLEAR,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;
unsigned long stateTimer = 0;

int level = 0;
int moves = 0;

int px = 0;
int py = 0;
int pAnim = 0;

char grid[ROWS][COLS];

const int NUM_LEVELS = 3;

// Here are 3 classic simple microban levels:
const char* microban[NUM_LEVELS][ROWS] = {
    {
        "  ####          ",
        "  #  #          ",
        "  #  #          ",
        "###  #          ",
        "# $  #          ",
        "# #. #          ",
        "# @  #          ",
        "######          "
    },
    {
        "######          ",
        "#    #          ",
        "# $. #          ",
        "##$@ #          ",
        " # . #          ",
        " #   #          ",
        " #####          ",
        "                "
    },
    {
        " #####          ",
        " #   #          ",
        " # * #          ",
        "##$ $#          ",
        "#  @ #          ",
        "######          ",
        "                ",
        "                "
    }
};

void loadLevel(int l) {
    for(int r=0; r<ROWS; r++) {
        for(int c=0; c<COLS; c++) {
            grid[r][c] = microban[l][r][c];
            if (grid[r][c] == '@') {
                px = c;
                py = r;
                grid[r][c] = ' '; // floor underneath
            } else if (grid[r][c] == '+') {
                px = c;
                py = r;
                grid[r][c] = '.';
            }
        }
    }
    moves = 0;
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

bool checkWin() {
    for(int r=0; r<ROWS; r++) {
        for(int c=0; c<COLS; c++) {
            if (grid[r][c] == '$') return false; // Unplaced crate
        }
    }
    return true; // All crates are on targets ('*')
}

unsigned long lastTime = 0;
unsigned long lastMoveTime = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    int dir = input_get_direction();
    bool resetBtn = input_reset();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(40, 20, "SOKOBAN");
        drawText(20, 40, "PRESS TO START");
        display_render();
        
        if (resetBtn) {
            level = 0;
            loadLevel(level);
            state = STATE_PLAYING;
            tone(BUZZER_PIN, 1000, 100);
        }
    } else if (state == STATE_PLAYING) {
        
        if (resetBtn) {
            loadLevel(level); // Restart level
            tone(BUZZER_PIN, 500, 200);
        }
        
        if (now - lastMoveTime > 200 && dir != -1) {
            int dx = 0;
            int dy = 0;
            if (dir == 0) dy = -1;
            if (dir == 1) dx = 1;
            if (dir == 2) dy = 1;
            if (dir == 3) dx = -1;
            
            int nx = px + dx;
            int ny = py + dy;
            
            if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS) {
                char target = grid[ny][nx];
                
                if (target == ' ' || target == '.') {
                    // Move
                    px = nx;
                    py = ny;
                    moves++;
                    lastMoveTime = now;
                    pAnim = (pAnim + 1) % 2;
                    tone(BUZZER_PIN, 800, 20);
                } else if (target == '$' || target == '*') {
                    // Try push
                    int nnx = nx + dx;
                    int nny = ny + dy;
                    if (nnx >= 0 && nnx < COLS && nny >= 0 && nny < ROWS) {
                        char ntarget = grid[nny][nnx];
                        if (ntarget == ' ' || ntarget == '.') {
                            // Push success
                            grid[ny][nx] = (target == '$') ? ' ' : '.'; // Leave behind
                            grid[nny][nnx] = (ntarget == ' ') ? '$' : '*'; // Place crate
                            px = nx;
                            py = ny;
                            moves++;
                            lastMoveTime = now;
                            pAnim = (pAnim + 1) % 2;
                            tone(BUZZER_PIN, 1200, 50);
                            
                            if (checkWin()) {
                                state = STATE_LEVEL_CLEAR;
                                stateTimer = now;
                                tone(BUZZER_PIN, 1500, 500);
                            }
                        }
                    }
                }
            }
        }
        
        // Render
        display_clear();
        
        for(int r=0; r<ROWS; r++) {
            for(int c=0; c<COLS; c++) {
                char t = grid[r][c];
                int tx = c * TILE;
                int ty = r * TILE;
                
                if (t == '#') {
                    fillRect(tx, ty, TILE, TILE, 1);
                    drawPixel(tx+2, ty+2, 0);
                    drawPixel(tx+5, ty+5, 0);
                } else if (t == '.') {
                    drawLine(tx+3, ty+3, tx+4, ty+4, 1);
                    drawLine(tx+4, ty+3, tx+3, ty+4, 1);
                } else if (t == '$') {
                    drawRect(tx+1, ty+1, TILE-2, TILE-2, 1);
                    drawLine(tx+2, ty+2, tx+TILE-3, ty+TILE-3, 1);
                } else if (t == '*') {
                    fillRect(tx+1, ty+1, TILE-2, TILE-2, 1);
                    drawPixel(tx+3, ty+3, 0); drawPixel(tx+4, ty+4, 0);
                }
            }
        }
        
        // Player
        int pDrawX = px * TILE;
        int pDrawY = py * TILE;
        drawCircle(pDrawX + 4, pDrawY + 4, 3, 1);
        if (pAnim == 0) drawPixel(pDrawX + 4, pDrawY + 4, 1);
        
        // HUD
        fillRect(100, 0, 28, 64, 0); // Side bar for HUD
        drawLine(100, 0, 100, 64, 1);
        drawText(105, 5, "LVL");
        drawText(105, 15, level + 1);
        drawText(105, 30, "MOV");
        drawText(105, 40, moves);
        
        display_render();
        
    } else if (state == STATE_LEVEL_CLEAR) {
        display_clear();
        drawText(30, 20, "LEVEL CLEAR!");
        drawText(35, 40, "MOVES:");
        drawText(75, 40, moves);
        display_render();
        
        if (now - stateTimer > 2000) {
            level++;
            if (level >= NUM_LEVELS) {
                state = STATE_GAMEOVER;
                stateTimer = now;
            } else {
                loadLevel(level);
                state = STATE_PLAYING;
            }
        }
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(20, 20, "YOU BEAT ALL");
        drawText(20, 30, "THE LEVELS!!");
        drawText(15, 50, "PRESS TO RESTART");
        display_render();
        
        if (resetBtn && now - stateTimer > 1000) {
            state = STATE_INTRO;
        }
    }
}

}
