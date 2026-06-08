#include "bomberboy_main.h"
#include "bomberboy_display.h"
#include "bomberboy_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Bomberboy {

const int SCREEN_W = 128;
const int SCREEN_H = 64;
const int TILE = 8;
const int COLS = 12;
const int ROWS = 8;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_LEVEL_CLEAR
};

GameState state = STATE_INTRO;
unsigned long stateTimer = 0;

int score = 0;
int level = 1;
int lives = 3;

char grid[ROWS][COLS];

// Player
struct Player {
    int x, y; // Grid coords
    bool alive;
};
Player p;

// Enemies
struct Enemy {
    int x, y;
    int dir; // 0=Up, 1=Right, 2=Down, 3=Left
    bool active;
};
const int MAX_ENEMIES = 5;
Enemy enemies[MAX_ENEMIES];

// Bombs
struct Bomb {
    int x, y;
    unsigned long timer;
    bool active;
};
const int MAX_BOMBS = 1; // Only 1 bomb at a time to start
Bomb bomb;

// Explosions
struct Blast {
    int x, y;
    unsigned long timer;
    bool active;
};
const int MAX_BLASTS = 20; // Enough for a cross shape
Blast blasts[MAX_BLASTS];

void spawnBlast(int x, int y, unsigned long now) {
    for(int i=0; i<MAX_BLASTS; i++) {
        if (!blasts[i].active) {
            blasts[i].active = true;
            blasts[i].x = x;
            blasts[i].y = y;
            blasts[i].timer = now;
            break;
        }
    }
}

void generateLevel() {
    for(int r=0; r<ROWS; r++) {
        for(int c=0; c<COLS; c++) {
            // Borders
            if (r == 0 || r == ROWS-1 || c == 0 || c == COLS-1) {
                grid[r][c] = '#';
            } 
            // Checkerboard pattern
            else if (r % 2 == 0 && c % 2 == 0) {
                grid[r][c] = '#';
            }
            else {
                // Soft blocks (bricks)
                if (random(0, 100) < 40) grid[r][c] = '+';
                else grid[r][c] = ' ';
            }
        }
    }
    
    // Clear spawn area
    grid[1][1] = ' ';
    grid[1][2] = ' ';
    grid[2][1] = ' ';
    p.x = 1; p.y = 1; p.alive = true;
    
    // Spawn enemies
    int eCount = 0;
    int targetE = min(level + 1, MAX_ENEMIES);
    for(int i=0; i<MAX_ENEMIES; i++) enemies[i].active = false;
    
    while(eCount < targetE) {
        int ex = random(3, COLS-1);
        int ey = random(3, ROWS-1);
        if (grid[ey][ex] == ' ') {
            enemies[eCount].active = true;
            enemies[eCount].x = ex;
            enemies[eCount].y = ey;
            enemies[eCount].dir = random(0, 4);
            eCount++;
        }
    }
    
    bomb.active = false;
    for(int i=0; i<MAX_BLASTS; i++) blasts[i].active = false;
}

void resetGame() {
    score = 0;
    level = 1;
    lives = 3;
    generateLevel();
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

unsigned long lastTime = 0;
unsigned long lastMoveTime = 0;
unsigned long lastEnemyMove = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    int dir = input_get_direction();
    bool bombBtn = input_bomb();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(35, 20, "BOMBERBOY");
        drawText(20, 40, "PRESS TO START");
        display_render();
        
        if (bombBtn) {
            resetGame();
            state = STATE_PLAYING;
            tone(BUZZER_PIN, 1500, 200);
        }
    } else if (state == STATE_PLAYING) {
        
        // --- Player Input ---
        if (now - lastMoveTime > 150 && dir != -1) {
            int dx = 0, dy = 0;
            if (dir == 0) dy = -1;
            else if (dir == 1) dx = 1;
            else if (dir == 2) dy = 1;
            else if (dir == 3) dx = -1;
            
            int nx = p.x + dx;
            int ny = p.y + dy;
            
            if (grid[ny][nx] == ' ') {
                // Check if moving into a bomb (prevent walking over it)
                if (bomb.active && bomb.x == nx && bomb.y == ny) {
                    // Blocked
                } else {
                    p.x = nx;
                    p.y = ny;
                    lastMoveTime = now;
                    tone(BUZZER_PIN, 600, 20); // Step
                }
            }
        }
        
        // Drop Bomb
        if (bombBtn && !bomb.active) {
            bomb.active = true;
            bomb.x = p.x;
            bomb.y = p.y;
            bomb.timer = now;
            tone(BUZZER_PIN, 800, 50);
        }
        
        // --- Bomb Logic ---
        if (bomb.active && now - bomb.timer > 2000) { // 2 second fuse
            bomb.active = false;
            tone(BUZZER_PIN, 200, 300); // Explode
            
            // Generate blasts (Center + 4 directions radius 2)
            int bx = bomb.x;
            int by = bomb.y;
            spawnBlast(bx, by, now);
            
            int dirs[4][2] = {{0,-1}, {1,0}, {0,1}, {-1,0}};
            for(int d=0; d<4; d++) {
                for(int i=1; i<=2; i++) { // Radius 2
                    int nx = bx + dirs[d][0] * i;
                    int ny = by + dirs[d][1] * i;
                    if (grid[ny][nx] == '#') break; // Stopped by hard wall
                    
                    if (grid[ny][nx] == '+') {
                        // Destroy soft wall and stop
                        grid[ny][nx] = ' ';
                        spawnBlast(nx, ny, now);
                        score += 50;
                        break;
                    }
                    
                    spawnBlast(nx, ny, now);
                }
            }
        }
        
        // --- Enemy Logic ---
        if (now - lastEnemyMove > 500 - (level * 20)) { // Faster enemies
            lastEnemyMove = now;
            for(int i=0; i<MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    int dx = 0, dy = 0;
                    if (enemies[i].dir == 0) dy = -1;
                    else if (enemies[i].dir == 1) dx = 1;
                    else if (enemies[i].dir == 2) dy = 1;
                    else if (enemies[i].dir == 3) dx = -1;
                    
                    int nx = enemies[i].x + dx;
                    int ny = enemies[i].y + dy;
                    
                    if (grid[ny][nx] == ' ' && !(bomb.active && bomb.x == nx && bomb.y == ny)) {
                        enemies[i].x = nx;
                        enemies[i].y = ny;
                        // Random turn chance
                        if (random(0, 10) < 2) enemies[i].dir = random(0, 4);
                    } else {
                        // Blocked, turn
                        enemies[i].dir = random(0, 4);
                    }
                }
            }
        }
        
        // --- Collision Check (Blast vs Player/Enemies) ---
        bool playerDied = false;
        
        for(int i=0; i<MAX_BLASTS; i++) {
            if (blasts[i].active) {
                // Clear old blasts
                if (now - blasts[i].timer > 500) {
                    blasts[i].active = false;
                    continue;
                }
                
                // Hit player
                if (p.x == blasts[i].x && p.y == blasts[i].y) {
                    playerDied = true;
                }
                
                // Hit enemies
                for(int e=0; e<MAX_ENEMIES; e++) {
                    if (enemies[e].active && enemies[e].x == blasts[i].x && enemies[e].y == blasts[i].y) {
                        enemies[e].active = false;
                        score += 100;
                        tone(BUZZER_PIN, 2500, 100);
                    }
                }
            }
        }
        
        // Hit by enemy
        for(int e=0; e<MAX_ENEMIES; e++) {
            if (enemies[e].active && p.x == enemies[e].x && p.y == enemies[e].y) {
                playerDied = true;
            }
        }
        
        if (playerDied) {
            lives--;
            stateTimer = now;
            tone(BUZZER_PIN, 100, 1000);
            if (lives <= 0) {
                state = STATE_GAMEOVER;
            } else {
                generateLevel(); // Restart level
            }
        }
        
        // Check Win
        bool anyEnemy = false;
        for(int e=0; e<MAX_ENEMIES; e++) if(enemies[e].active) anyEnemy = true;
        if (!anyEnemy) {
            level++;
            state = STATE_LEVEL_CLEAR;
            stateTimer = now;
            tone(BUZZER_PIN, 1500, 1000);
        }
        
        // --- Render ---
        display_clear();
        
        for(int r=0; r<ROWS; r++) {
            for(int c=0; c<COLS; c++) {
                int tx = c * TILE;
                int ty = r * TILE;
                if (grid[r][c] == '#') {
                    fillRect(tx, ty, TILE, TILE, 1);
                } else if (grid[r][c] == '+') {
                    drawRect(tx, ty, TILE, TILE, 1);
                    drawLine(tx, ty+4, tx+8, ty+4, 1);
                    drawLine(tx+4, ty, tx+4, ty+4, 1);
                }
            }
        }
        
        // Bomb
        if (bomb.active) {
            int tx = bomb.x * TILE;
            int ty = bomb.y * TILE;
            if ((now / 100) % 2 == 0) {
                fillCircle(tx+4, ty+4, 3, 1); // Pulse
            } else {
                drawCircle(tx+4, ty+4, 3, 1);
            }
        }
        
        // Blasts
        for(int i=0; i<MAX_BLASTS; i++) {
            if (blasts[i].active) {
                int tx = blasts[i].x * TILE;
                int ty = blasts[i].y * TILE;
                drawLine(tx, ty, tx+8, ty+8, 1);
                drawLine(tx+8, ty, tx, ty+8, 1);
                drawLine(tx+4, ty, tx+4, ty+8, 1);
                drawLine(tx, ty+4, tx+8, ty+4, 1);
            }
        }
        
        // Enemies
        for(int i=0; i<MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                int tx = enemies[i].x * TILE;
                int ty = enemies[i].y * TILE;
                fillRect(tx+1, ty+2, 6, 6, 1);
                drawPixel(tx+2, ty+4, 0); // Eyes
                drawPixel(tx+5, ty+4, 0);
            }
        }
        
        // Player
        int px = p.x * TILE;
        int py = p.y * TILE;
        drawRect(px+1, py+1, 6, 6, 1);
        drawLine(px+4, py+3, px+4, py+7, 1);
        
        // HUD
        fillRect(COLS * TILE, 0, SCREEN_W - (COLS * TILE), SCREEN_H, 0); // Clear sidebar
        drawLine(COLS * TILE, 0, COLS * TILE, SCREEN_H, 1); // Divider
        
        drawText(100, 2, "LVL");
        drawText(100, 12, level);
        drawText(100, 26, "SCR");
        drawText(100, 36, score);
        drawText(100, 50, "LIF");
        for(int i=0; i<lives; i++) {
            drawPixel(100 + i*5, 60, 1);
            drawPixel(101 + i*5, 60, 1);
        }
        
        display_render();
        
    } else if (state == STATE_LEVEL_CLEAR) {
        display_clear();
        drawText(20, 20, "STAGE CLEAR!");
        drawText(30, 40, "BONUS:"); drawText(70, 40, level * 100);
        display_render();
        
        if (now - stateTimer > 2000) {
            score += level * 100;
            generateLevel();
            state = STATE_PLAYING;
        }
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(35, 20, "GAME OVER");
        drawText(20, 40, "SCORE:");
        drawText(60, 40, score);
        display_render();
        
        if (bombBtn && now - stateTimer > 1000) {
            state = STATE_INTRO;
        }
    }
}

}
