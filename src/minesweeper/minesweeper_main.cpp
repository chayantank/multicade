#include "minesweeper_main.h"
#include "minesweeper_display.h"
#include "minesweeper_input.h"

#define BUZZER_PIN 14

namespace Minesweeper {

const int SCREEN_W = 128;
const int SCREEN_H = 64;

const int TILE_SIZE = 8;
const int COLS = 12;
const int ROWS = 6;
const int TOTAL_MINES = 10;

// Board offset to center it (12*8 = 96 wide, 6*8 = 48 high)
// Offset X: 0 (leaves 32 px on right for HUD)
// Offset Y: 8 (leaves 8 px top and 8 px bottom)
const int OFFSET_X = 2; 
const int OFFSET_Y = 8;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_DEATH,
    STATE_WIN
};

GameState state = STATE_INTRO;

struct Tile {
    bool isMine;
    bool isRevealed;
    bool isFlagged;
    int neighbors;
};

Tile grid[COLS][ROWS];

int cursorX = 0;
int cursorY = 0;
unsigned long lastMoveTime = 0;
unsigned long stateTimer = 0;

int flagsPlaced = 0;
int tilesRevealed = 0;
bool firstClick = true;

void calculateNeighbors() {
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            if (grid[x][y].isMine) continue;
            int count = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS) {
                        if (grid[nx][ny].isMine) count++;
                    }
                }
            }
            grid[x][y].neighbors = count;
        }
    }
}

void generateMines(int excludeX, int excludeY) {
    int minesPlaced = 0;
    while (minesPlaced < TOTAL_MINES) {
        int rx = random(0, COLS);
        int ry = random(0, ROWS);
        
        // Don't place on the first click tile, or immediate neighbors if possible
        // Actually, just ensuring it's not the exact clicked tile is enough for basic Minesweeper
        if (rx == excludeX && ry == excludeY) continue;
        
        if (!grid[rx][ry].isMine) {
            grid[rx][ry].isMine = true;
            minesPlaced++;
        }
    }
    calculateNeighbors();
}

void resetLevel() {
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            grid[x][y].isMine = false;
            grid[x][y].isRevealed = false;
            grid[x][y].isFlagged = false;
            grid[x][y].neighbors = 0;
        }
    }
    cursorX = COLS / 2;
    cursorY = ROWS / 2;
    flagsPlaced = 0;
    tilesRevealed = 0;
    firstClick = true;
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

void revealTile(int x, int y) {
    if (x < 0 || x >= COLS || y < 0 || y >= ROWS) return;
    if (grid[x][y].isRevealed || grid[x][y].isFlagged) return;
    
    grid[x][y].isRevealed = true;
    tilesRevealed++;
    
    if (grid[x][y].neighbors == 0 && !grid[x][y].isMine) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx != 0 || dy != 0) {
                    revealTile(x + dx, y + dy);
                }
            }
        }
    }
}

void checkWin() {
    if (tilesRevealed == (COLS * ROWS) - TOTAL_MINES) {
        state = STATE_WIN;
        tone(BUZZER_PIN, 1500, 500);
    }
}

void loopPlaying() {
    // Movement logic (with delay so cursor doesn't fly)
    if (millis() - lastMoveTime > 150) {
        bool moved = false;
        if (input_up() && cursorY > 0) { cursorY--; moved = true; }
        else if (input_down() && cursorY < ROWS - 1) { cursorY++; moved = true; }
        else if (input_left() && cursorX > 0) { cursorX--; moved = true; }
        else if (input_right() && cursorX < COLS - 1) { cursorX++; moved = true; }
        
        if (moved) {
            lastMoveTime = millis();
            // Optional tiny blip
            // tone(BUZZER_PIN, 400, 5); 
        }
    }
    
    Action act = input_action();
    
    if (act == ACTION_FLAG) {
        if (!grid[cursorX][cursorY].isRevealed) {
            grid[cursorX][cursorY].isFlagged = !grid[cursorX][cursorY].isFlagged;
            if (grid[cursorX][cursorY].isFlagged) flagsPlaced++;
            else flagsPlaced--;
        }
    } else if (act == ACTION_REVEAL) {
        if (!grid[cursorX][cursorY].isFlagged && !grid[cursorX][cursorY].isRevealed) {
            if (firstClick) {
                generateMines(cursorX, cursorY);
                firstClick = false;
            }
            
            if (grid[cursorX][cursorY].isMine) {
                // Death
                grid[cursorX][cursorY].isRevealed = true; // Show the explosive
                state = STATE_DEATH;
                stateTimer = millis();
                tone(BUZZER_PIN, 100, 1000);
            } else {
                revealTile(cursorX, cursorY);
                tone(BUZZER_PIN, 800, 30);
                checkWin();
            }
        }
    }
    
    // Draw
    display_clear();
    
    // Draw Board
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            int px = OFFSET_X + x * TILE_SIZE;
            int py = OFFSET_Y + y * TILE_SIZE;
            
            drawRect(px, py, TILE_SIZE, TILE_SIZE, 1);
            
            if (grid[x][y].isRevealed) {
                if (grid[x][y].isMine) {
                    drawMine(px, py);
                } else if (grid[x][y].neighbors > 0) {
                    drawText(px + 2, py + 1, grid[x][y].neighbors);
                } else {
                    // Empty revealed tile is just a box with a dither pattern or empty
                    fillRect(px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2, 0); // Clear inside
                }
            } else {
                // Unrevealed tile looks filled
                fillRect(px + 2, py + 2, TILE_SIZE - 4, TILE_SIZE - 4, 1);
                
                if (grid[x][y].isFlagged) {
                    // Draw flag over the unrevealed tile
                    fillRect(px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2, 0); // Clear inside first
                    drawFlag(px, py);
                }
            }
        }
    }
    
    // Draw Cursor
    int cx = OFFSET_X + cursorX * TILE_SIZE;
    int cy = OFFSET_Y + cursorY * TILE_SIZE;
    // Invert the cursor box to make it blink or stand out
    if ((millis() / 250) % 2 == 0) {
        drawRect(cx, cy, TILE_SIZE, TILE_SIZE, 0);
        drawRect(cx+1, cy+1, TILE_SIZE-2, TILE_SIZE-2, 1);
    }
    
    // Draw HUD
    int hudX = OFFSET_X + COLS * TILE_SIZE + 4;
    drawText(hudX, 8, "MINES");
    drawText(hudX, 18, TOTAL_MINES - flagsPlaced);
    
    display_render();
}

void loop() {
    if (state == STATE_INTRO) {
        display_clear();
        drawText(30, 10, "MINESWEEPER");
        drawText(5, 30, "Click: Reveal");
        drawText(5, 40, "Hold : Flag");
        drawText(5, 50, "PRESS TO START");
        display_render();

        if (input_action() == ACTION_REVEAL) {
            resetLevel();
            state = STATE_PLAYING;
        }
    } else if (state == STATE_PLAYING) {
        loopPlaying();
    } else if (state == STATE_DEATH) {
        // Draw one last time with game over text
        display_clear();
        
        // Render board but reveal all mines
        for (int y = 0; y < ROWS; y++) {
            for (int x = 0; x < COLS; x++) {
                int px = OFFSET_X + x * TILE_SIZE;
                int py = OFFSET_Y + y * TILE_SIZE;
                drawRect(px, py, TILE_SIZE, TILE_SIZE, 1);
                
                if (grid[x][y].isMine) {
                    fillRect(px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2, 0); 
                    drawMine(px, py);
                } else if (grid[x][y].isRevealed && grid[x][y].neighbors > 0) {
                    drawText(px + 2, py + 1, grid[x][y].neighbors);
                } else if (!grid[x][y].isRevealed) {
                    fillRect(px + 2, py + 2, TILE_SIZE - 4, TILE_SIZE - 4, 1);
                    if (grid[x][y].isFlagged) {
                        fillRect(px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2, 0);
                        drawFlag(px, py);
                        // Strike through wrong flags
                        drawLine(px, py, px + TILE_SIZE, py + TILE_SIZE, 1);
                    }
                }
            }
        }
        
        fillRect(14, 24, 100, 16, 0);
        drawRect(14, 24, 100, 16, 1);
        drawText(38, 28, "BOOM!");
        display_render();

        if (millis() - stateTimer > 2000) {
            if (input_action() == ACTION_REVEAL) {
                state = STATE_INTRO;
            }
        } else {
            // Drain queue
            input_action();
        }
    } else if (state == STATE_WIN) {
        display_clear();
        drawText(35, 20, "YOU WIN!");
        display_render();

        if (input_action() == ACTION_REVEAL) {
            state = STATE_INTRO;
        }
    }
}

}
