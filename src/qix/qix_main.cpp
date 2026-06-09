#include "qix_main.h"
#include "qix_display.h"
#include "qix_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Qix {

const int GRID_W = 30;
const int GRID_H = 14;
const int CELL_SIZE = 4;
const int OFFSET_X = 4;
const int OFFSET_Y = 4;

uint8_t grid[GRID_W][GRID_H];

float px = 0, py = 0;
int moveDelay = 0;
bool drawing = false;

struct Spark {
    float x, y;
    float vx, vy;
};
Spark spark;

int score = 0;
bool gameOver = false;
bool levelClear = false;
int lives = 3;

void resetLevel() {
    for(int x=0; x<GRID_W; x++) {
        for(int y=0; y<GRID_H; y++) {
            if (x==0 || x==GRID_W-1 || y==0 || y==GRID_H-1) grid[x][y] = 1;
            else grid[x][y] = 0;
        }
    }
    px = 0; py = 0;
    drawing = false;
    spark = {15, 7, 0.5f, 0.5f};
    gameOver = false;
    levelClear = false;
}

void setup() {
    display_setup();
    input_setup();
    score = 0;
    lives = 3;
    resetLevel();
}

void floodFill(int x, int y, uint8_t target, uint8_t replacement) {
    if (x<0 || x>=GRID_W || y<0 || y>=GRID_H) return;
    if (grid[x][y] != target) return;
    grid[x][y] = replacement;
    floodFill(x+1, y, target, replacement);
    floodFill(x-1, y, target, replacement);
    floodFill(x, y+1, target, replacement);
    floodFill(x, y-1, target, replacement);
}

void captureArea() {
    for(int x=0; x<GRID_W; x++) {
        for(int y=0; y<GRID_H; y++) {
            if (grid[x][y] == 3) grid[x][y] = 1;
        }
    }
    
    int sx = (int)spark.x;
    int sy = (int)spark.y;
    
    floodFill(sx, sy, 0, 5);
    
    int capturedCount = 0;
    for(int x=0; x<GRID_W; x++) {
        for(int y=0; y<GRID_H; y++) {
            if (grid[x][y] == 0) {
                grid[x][y] = 1;
                capturedCount++;
            }
            if (grid[x][y] == 5) {
                grid[x][y] = 0;
            }
        }
    }
    
    score += capturedCount * 10;
    tone(BUZZER_PIN, 1500, 100);
    
    int totalEmpty = 0;
    for(int x=0; x<GRID_W; x++) {
        for(int y=0; y<GRID_H; y++) {
            if (grid[x][y] == 0) totalEmpty++;
        }
    }
    
    if (totalEmpty < (GRID_W-2)*(GRID_H-2) * 0.25f) {
        levelClear = true;
        tone(BUZZER_PIN, 800, 100); delay(100); tone(BUZZER_PIN, 1200, 100);
    }
}

unsigned long lastTime = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.05f) dt = 0.05f;
    
    if (gameOver) {
        display_clear();
        drawText(35, 20, "GAME OVER");
        display_render();
        if (input_action() && now % 500 < 50) {
            score = 0;
            lives = 3;
            resetLevel();
        }
        return;
    }
    
    if (levelClear) {
        display_clear();
        drawText(30, 20, "STAGE CLEAR");
        display_render();
        if (input_action() && now % 500 < 50) {
            resetLevel();
        }
        return;
    }
    
    moveDelay--;
    if (moveDelay <= 0) {
        int jx = input_x() - 2048;
        int jy = input_y() - 2048;
        
        int dx = 0, dy = 0;
        if (abs(jx) > 1000) dx = (jx > 0) ? 1 : -1;
        else if (abs(jy) > 1000) dy = (jy > 0) ? 1 : -1;
        
        if (dx != 0 || dy != 0) {
            int nx = (int)px + dx;
            int ny = (int)py + dy;
            
            if (nx >= 0 && nx < GRID_W && ny >= 0 && ny < GRID_H) {
                bool action = input_action();
                
                if (action) {
                    if (grid[nx][ny] == 0) {
                        px = nx; py = ny;
                        grid[(int)px][(int)py] = 3;
                        drawing = true;
                        tone(BUZZER_PIN, 400, 10);
                    } else if (grid[nx][ny] == 1 && drawing) {
                        px = nx; py = ny;
                        drawing = false;
                        captureArea();
                    } else if (grid[nx][ny] == 1 && !drawing) {
                        px = nx; py = ny;
                    }
                } else {
                    if (grid[nx][ny] == 1 && !drawing) {
                        px = nx; py = ny;
                    }
                }
            }
            moveDelay = 2;
        }
    }
    
    spark.x += spark.vx * dt * 15.0f;
    spark.y += spark.vy * dt * 15.0f;
    
    int sx = (int)spark.x;
    int sy = (int)spark.y;
    if (sx < 0 || sx >= GRID_W || sy < 0 || sy >= GRID_H || grid[sx][sy] == 1) {
        spark.vx = -spark.vx;
        spark.x += spark.vx * 2;
        spark.vy += ((random(0,100)-50)/100.0f) * 0.1f;
    }
    if (sy < 0 || sy >= GRID_H || grid[sx][sy] == 1) {
        spark.vy = -spark.vy;
        spark.y += spark.vy * 2;
        spark.vx += ((random(0,100)-50)/100.0f) * 0.1f;
    }
    
    float len = sqrt(spark.vx*spark.vx + spark.vy*spark.vy);
    spark.vx = (spark.vx/len) * 0.5f;
    spark.vy = (spark.vy/len) * 0.5f;
    
    if (sx >= 0 && sx < GRID_W && sy >= 0 && sy < GRID_H && grid[sx][sy] == 3) {
        lives--;
        tone(BUZZER_PIN, 100, 500);
        if (lives <= 0) gameOver = true;
        else {
            for(int x=0; x<GRID_W; x++) {
                for(int y=0; y<GRID_H; y++) {
                    if (grid[x][y] == 3) grid[x][y] = 0;
                }
            }
            drawing = false;
            px = 0; py = 0;
        }
    }
    
    display_clear();
    
    for(int x=0; x<GRID_W; x++) {
        for(int y=0; y<GRID_H; y++) {
            if (grid[x][y] == 1) {
                fillRect(OFFSET_X + x*CELL_SIZE, OFFSET_Y + y*CELL_SIZE, CELL_SIZE, CELL_SIZE, 1);
            } else if (grid[x][y] == 3) {
                drawRect(OFFSET_X + x*CELL_SIZE, OFFSET_Y + y*CELL_SIZE, CELL_SIZE, CELL_SIZE, 1);
            }
        }
    }
    
    fillRect(OFFSET_X + (int)px*CELL_SIZE, OFFSET_Y + (int)py*CELL_SIZE, CELL_SIZE, CELL_SIZE, 0);
    drawCircle(OFFSET_X + (int)px*CELL_SIZE + 2, OFFSET_Y + (int)py*CELL_SIZE + 2, 1, 1);
    
    drawCircle(OFFSET_X + (int)spark.x*CELL_SIZE + 2, OFFSET_Y + (int)spark.y*CELL_SIZE + 2, 2, 1);
    
    drawText(0, 0, "S:"); drawText(12, 0, score);
    drawText(90, 0, "L:"); drawText(102, 0, lives);
    
    display_render();
}

}
