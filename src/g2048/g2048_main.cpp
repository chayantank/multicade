#include "g2048_main.h"
#include "g2048_display.h"
#include "g2048_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace G2048 {

int grid[4][4];
int score = 0;
bool gameOver = false;
bool win = false;

void spawnTile() {
    int emptyCount = 0;
    for(int r=0; r<4; r++) {
        for(int c=0; c<4; c++) {
            if (grid[r][c] == 0) emptyCount++;
        }
    }
    
    if (emptyCount == 0) return;
    
    int spawnIdx = random(0, emptyCount);
    int count = 0;
    for(int r=0; r<4; r++) {
        for(int c=0; c<4; c++) {
            if (grid[r][c] == 0) {
                if (count == spawnIdx) {
                    grid[r][c] = (random(0, 10) < 9) ? 2 : 4;
                    return;
                }
                count++;
            }
        }
    }
}

bool checkGameOver() {
    for(int r=0; r<4; r++) {
        for(int c=0; c<4; c++) {
            if (grid[r][c] == 0) return false;
            if (r < 3 && grid[r][c] == grid[r+1][c]) return false;
            if (c < 3 && grid[r][c] == grid[r][c+1]) return false;
        }
    }
    return true;
}

void resetGame() {
    for(int r=0; r<4; r++) {
        for(int c=0; c<4; c++) {
            grid[r][c] = 0;
        }
    }
    score = 0;
    gameOver = false;
    win = false;
    spawnTile();
    spawnTile();
}

void setup() {
    display_setup();
    input_setup();
    resetGame();
}

bool slide(int dr, int dc) {
    bool moved = false;
    bool merged[4][4] = {false};
    
    // Determine iteration order
    int rStart = (dr == 1) ? 3 : 0;
    int rEnd = (dr == 1) ? -1 : 4;
    int rStep = (dr == 1) ? -1 : 1;
    
    int cStart = (dc == 1) ? 3 : 0;
    int cEnd = (dc == 1) ? -1 : 4;
    int cStep = (dc == 1) ? -1 : 1;
    
    for(int r = rStart; r != rEnd; r += rStep) {
        for(int c = cStart; c != cEnd; c += cStep) {
            if (grid[r][c] != 0) {
                int currR = r;
                int currC = c;
                while(true) {
                    int nextR = currR + dr;
                    int nextC = currC + dc;
                    
                    if (nextR < 0 || nextR >= 4 || nextC < 0 || nextC >= 4) break;
                    
                    if (grid[nextR][nextC] == 0) {
                        grid[nextR][nextC] = grid[currR][currC];
                        grid[currR][currC] = 0;
                        currR = nextR;
                        currC = nextC;
                        moved = true;
                    } else if (grid[nextR][nextC] == grid[currR][currC] && !merged[nextR][nextC]) {
                        grid[nextR][nextC] *= 2;
                        grid[currR][currC] = 0;
                        merged[nextR][nextC] = true;
                        score += grid[nextR][nextC];
                        if (grid[nextR][nextC] == 2048) win = true;
                        moved = true;
                        break;
                    } else {
                        break; // different value or already merged
                    }
                }
            }
        }
    }
    return moved;
}

void loop() {
    if (gameOver) {
        display_clear();
        drawText(35, 20, "GAME OVER");
        drawText(40, 30, score);
        drawText(15, 50, "PRESS TO RESTART");
        display_render();
        if (input_action()) {
            tone(BUZZER_PIN, 800, 100); delay(200);
            resetGame();
        }
        return;
    }
    
    if (win) {
        display_clear();
        drawText(35, 20, "YOU WIN!");
        drawText(40, 30, score);
        drawText(15, 50, "PRESS TO CONT");
        display_render();
        if (input_action()) {
            tone(BUZZER_PIN, 1000, 200); delay(250); tone(BUZZER_PIN, 1500, 300);
            win = false; // continue playing
        }
        return;
    }
    
    bool moved = false;
    
    if (input_up()) moved = slide(-1, 0);
    else if (input_down()) moved = slide(1, 0);
    else if (input_left()) moved = slide(0, -1);
    else if (input_right()) moved = slide(0, 1);
    
    if (moved) {
        spawnTile();
        tone(BUZZER_PIN, 800, 30);
        if (checkGameOver()) {
            gameOver = true;
            tone(BUZZER_PIN, 200, 500);
        }
    }
    
    // Render
    display_clear();
    
    // Draw Grid borders
    drawRect(34, 2, 60, 60, 1);
    
    for(int r=0; r<4; r++) {
        for(int c=0; c<4; c++) {
            drawTile(r, c, grid[r][c]);
        }
    }
    
    // UI
    drawText(2, 2, "SCR:");
    drawText(2, 12, score);
    
    display_render();
    
    // Quick delay to prevent multi-inputs
    if (moved) delay(150);
}

}
