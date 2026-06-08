#include "bomber_main.h"
#include "bomber_display.h"
#include "bomber_input.h"

#define BUZZER_PIN 14

namespace Bomber {

const int COLS = 15;
const int ROWS = 7;
const int TILE = 8;
const int OFFSET_X = 4;
const int OFFSET_Y = 8; // Leave room for HUD

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_WIN
};

GameState state = STATE_INTRO;
unsigned long stateTimer = 0;

// 0=Air, 1=Hard, 2=Soft, 3=Powerup(Radius), 4=Exit
uint8_t grid[COLS][ROWS];

float px = 1 * TILE;
float py = 1 * TILE;
int blastRadius = 1;
bool isDead = false;

struct Bomb {
    int tx, ty;
    unsigned long timer;
    bool active;
};
Bomb bomb;

const int MAX_FLAMES = 20;
struct Flame {
    int tx, ty;
    unsigned long timer;
    bool active;
} flames[MAX_FLAMES];

const int MAX_ENEMIES = 3;
struct Enemy {
    float x, y;
    int dir; // 0=up, 1=right, 2=down, 3=left
    bool active;
} enemies[MAX_ENEMIES];

void initMap() {
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            if (x == 0 || x == COLS-1 || y == 0 || y == ROWS-1) grid[x][y] = 1; // Border
            else if (x % 2 == 0 && y % 2 == 0) grid[x][y] = 1; // Hard blocks
            else {
                // Soft blocks (except top-left corner)
                if ((x==1 && y==1) || (x==2 && y==1) || (x==1 && y==2)) grid[x][y] = 0;
                else {
                    if (random(0, 100) < 40) grid[x][y] = 2;
                    else grid[x][y] = 0;
                }
            }
        }
    }
    
    // Hide an exit
    bool exitHidden = false;
    while (!exitHidden) {
        int ex = random(1, COLS-1);
        int ey = random(1, ROWS-1);
        if (grid[ex][ey] == 2) {
            grid[ex][ey] = 4; // Exit
            exitHidden = true;
        }
    }
    
    // Hide a powerup
    bool powerupHidden = false;
    while (!powerupHidden) {
        int px = random(1, COLS-1);
        int py = random(1, ROWS-1);
        if (grid[px][py] == 2) {
            grid[px][py] = 3; // Powerup
            powerupHidden = true;
        }
    }
    
    // Enemies
    for (int i=0; i<MAX_ENEMIES; i++) {
        enemies[i].active = true;
        enemies[i].x = (COLS - 2) * TILE;
        enemies[i].y = (ROWS - 2) * TILE;
        enemies[i].dir = 1;
    }
    
    px = 1 * TILE;
    py = 1 * TILE;
    blastRadius = 1;
    isDead = false;
    bomb.active = false;
    for(int i=0; i<MAX_FLAMES; i++) flames[i].active = false;
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

void spawnFlame(int tx, int ty, unsigned long now) {
    for(int i=0; i<MAX_FLAMES; i++) {
        if (!flames[i].active) {
            flames[i].active = true;
            flames[i].tx = tx;
            flames[i].ty = ty;
            flames[i].timer = now;
            break;
        }
    }
}

void explodeBomb(unsigned long now) {
    bomb.active = false;
    tone(BUZZER_PIN, 100, 300);
    
    int bx = bomb.tx;
    int by = bomb.ty;
    
    spawnFlame(bx, by, now); // Center
    
    int dirs[4][2] = {{0,-1}, {1,0}, {0,1}, {-1,0}};
    for (int d = 0; d < 4; d++) {
        for (int r = 1; r <= blastRadius; r++) {
            int nx = bx + dirs[d][0] * r;
            int ny = by + dirs[d][1] * r;
            
            if (grid[nx][ny] == 1) break; // Hard wall stops blast
            
            if (grid[nx][ny] >= 2) { // Soft wall, powerup, or exit
                if (grid[nx][ny] == 2) grid[nx][ny] = 0; // Destroy wall
                // If it was an exit or powerup, reveal it by removing the soft shell (actually, my logic replaced it). 
                // Let's refine: If we hit 3 or 4, do we destroy them? In real Bomberman, hitting powerups destroys them!
                // To keep it simple, hitting 3/4 destroys them too.
                if (grid[nx][ny] == 3 || grid[nx][ny] == 4) grid[nx][ny] = 0;
                
                spawnFlame(nx, ny, now);
                break; // Flame stops after hitting a block
            }
            
            spawnFlame(nx, ny, now);
        }
    }
}

unsigned long lastTime = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    bool fire = input_fire();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(15, 20, "BOMBARDIER");
        drawText(20, 40, "PRESS TO START");
        display_render();
        
        if (fire) {
            initMap();
            state = STATE_PLAYING;
        }
    } else if (state == STATE_PLAYING) {
        
        // --- Player Input ---
        int dir = input_get_direction();
        float nx = px;
        float ny = py;
        float speed = 40.0f;
        
        if (dir == 0) ny -= speed * dt;
        else if (dir == 1) nx += speed * dt;
        else if (dir == 2) ny += speed * dt;
        else if (dir == 3) nx -= speed * dt;
        
        // Collision (check corners of hitbox)
        int left = (nx + 1) / TILE;
        int right = (nx + TILE - 2) / TILE;
        int top = (ny + 1) / TILE;
        int bottom = (ny + TILE - 2) / TILE;
        
        bool blocked = false;
        if (grid[left][top] == 1 || grid[left][top] == 2 || grid[left][top] == 3 || grid[left][top] == 4) blocked = true;
        if (grid[right][top] == 1 || grid[right][top] == 2 || grid[right][top] == 3 || grid[right][top] == 4) blocked = true;
        if (grid[left][bottom] == 1 || grid[left][bottom] == 2 || grid[left][bottom] == 3 || grid[left][bottom] == 4) blocked = true;
        if (grid[right][bottom] == 1 || grid[right][bottom] == 2 || grid[right][bottom] == 3 || grid[right][bottom] == 4) blocked = true;
        
        if (!blocked) {
            px = nx;
            py = ny;
        }
        
        // Check powerups / exit (using center of player)
        int ctx = (px + TILE/2) / TILE;
        int cty = (py + TILE/2) / TILE;
        
        // If the grid was previously 3 or 4 but is now empty because it was blown up, we can't collect it.
        // Wait, if grid holds 3 or 4 directly, they block movement!
        // Ah, in Bomberman, powerups don't block movement. 
        // Let's change collision logic: 3 and 4 do NOT block movement.
        
        if (grid[ctx][cty] == 3) {
            blastRadius++;
            grid[ctx][cty] = 0;
            tone(BUZZER_PIN, 1200, 100);
        } else if (grid[ctx][cty] == 4) {
            // Check if enemies are dead
            bool enemiesDead = true;
            for(int i=0; i<MAX_ENEMIES; i++) if(enemies[i].active) enemiesDead = false;
            
            if (enemiesDead) {
                state = STATE_WIN;
                stateTimer = now;
                tone(BUZZER_PIN, 2000, 500);
            }
        }
        
        // Place Bomb
        if (fire && !bomb.active) {
            bomb.active = true;
            bomb.tx = (px + TILE/2) / TILE;
            bomb.ty = (py + TILE/2) / TILE;
            bomb.timer = now;
            tone(BUZZER_PIN, 600, 30);
        }
        
        // Update Bomb
        if (bomb.active && now - bomb.timer > 2500) {
            explodeBomb(now);
        }
        
        // Update Flames
        for(int i=0; i<MAX_FLAMES; i++) {
            if (flames[i].active) {
                if (now - flames[i].timer > 500) {
                    flames[i].active = false;
                } else {
                    // Check kill player
                    if (ctx == flames[i].tx && cty == flames[i].ty) {
                        isDead = true;
                        state = STATE_GAMEOVER;
                        stateTimer = now;
                        tone(BUZZER_PIN, 100, 800);
                    }
                    // Check kill enemy
                    for(int e=0; e<MAX_ENEMIES; e++) {
                        if (enemies[e].active) {
                            int etx = (enemies[e].x + TILE/2) / TILE;
                            int ety = (enemies[e].y + TILE/2) / TILE;
                            if (etx == flames[i].tx && ety == flames[i].ty) {
                                enemies[e].active = false;
                                tone(BUZZER_PIN, 800, 50);
                            }
                        }
                    }
                }
            }
        }
        
        // Update Enemies
        for(int i=0; i<MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                float ex = enemies[i].x;
                float ey = enemies[i].y;
                float espd = 20.0f;
                
                if (enemies[i].dir == 0) ey -= espd * dt;
                else if (enemies[i].dir == 1) ex += espd * dt;
                else if (enemies[i].dir == 2) ey += espd * dt;
                else if (enemies[i].dir == 3) ex -= espd * dt;
                
                // Collision
                int el = (ex + 1) / TILE;
                int er = (ex + TILE - 2) / TILE;
                int et = (ey + 1) / TILE;
                int eb = (ey + TILE - 2) / TILE;
                
                bool eBlocked = false;
                if (grid[el][et] == 1 || grid[el][et] == 2) eBlocked = true;
                if (grid[er][et] == 1 || grid[er][et] == 2) eBlocked = true;
                if (grid[el][eb] == 1 || grid[el][eb] == 2) eBlocked = true;
                if (grid[er][eb] == 1 || grid[er][eb] == 2) eBlocked = true;
                
                // Bombs block enemies
                if (bomb.active) {
                    if (el == bomb.tx && et == bomb.ty) eBlocked = true;
                    if (er == bomb.tx && et == bomb.ty) eBlocked = true;
                }
                
                if (eBlocked) {
                    // Turn around or random
                    enemies[i].dir = random(0, 4);
                    // Snap to grid to prevent getting stuck
                    enemies[i].x = ((int)(enemies[i].x / TILE)) * TILE;
                    enemies[i].y = ((int)(enemies[i].y / TILE)) * TILE;
                } else {
                    enemies[i].x = ex;
                    enemies[i].y = ey;
                }
                
                // Hit player
                if (abs(px - enemies[i].x) < TILE-2 && abs(py - enemies[i].y) < TILE-2) {
                    isDead = true;
                    state = STATE_GAMEOVER;
                    stateTimer = now;
                    tone(BUZZER_PIN, 100, 800);
                }
            }
        }
        
        // --- Render ---
        display_clear();
        
        // Draw HUD
        drawText(0, 0, "RAD:");
        drawText(25, 0, blastRadius);
        
        for (int y = 0; y < ROWS; y++) {
            for (int x = 0; x < COLS; x++) {
                int sx = x * TILE + OFFSET_X;
                int sy = y * TILE + OFFSET_Y;
                
                if (grid[x][y] == 1) { // Hard
                    fillRect(sx, sy, TILE, TILE, 1);
                } else if (grid[x][y] == 2) { // Soft
                    drawRect(sx, sy, TILE, TILE, 1);
                    drawLine(sx+2, sy+2, sx+5, sy+5, 1);
                } else if (grid[x][y] == 3) { // Powerup
                    drawCircle(sx+4, sy+4, 2, 1);
                } else if (grid[x][y] == 4) { // Exit
                    drawRect(sx+2, sy+2, 4, 4, 1);
                    drawPixel(sx+4, sy+4, 1);
                }
            }
        }
        
        // Draw Bomb
        if (bomb.active) {
            int bx = bomb.tx * TILE + OFFSET_X;
            int by = bomb.ty * TILE + OFFSET_Y;
            fillCircle(bx+4, by+4, 3, 1);
            if ((now / 100) % 2 == 0) drawLine(bx+4, by, bx+4, by-2, 1); // Spark
        }
        
        // Draw Flames
        for(int i=0; i<MAX_FLAMES; i++) {
            if (flames[i].active) {
                int fx = flames[i].tx * TILE + OFFSET_X;
                int fy = flames[i].ty * TILE + OFFSET_Y;
                // cross pattern
                drawLine(fx+4, fy, fx+4, fy+8, 1);
                drawLine(fx, fy+4, fx+8, fy+4, 1);
                // sparkle
                if (random(0,2)) drawPixel(fx+2, fy+2, 1);
            }
        }
        
        // Draw Enemies
        for(int i=0; i<MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                int ex = enemies[i].x + OFFSET_X;
                int ey = enemies[i].y + OFFSET_Y;
                fillRect(ex+1, ey+2, 6, 4, 1);
                drawPixel(ex+2, ey+3, 0); // Eyes
                drawPixel(ex+5, ey+3, 0);
            }
        }
        
        // Draw Player
        if (!isDead) {
            int pxx = px + OFFSET_X;
            int pyy = py + OFFSET_Y;
            drawCircle(pxx+4, pyy+4, 3, 1);
            drawLine(pxx+4, pyy+4, pxx+4, pyy+7, 1); // Body
        }
        
        display_render();
        
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(35, 30, "YOU DIED");
        display_render();
        if (fire && now - stateTimer > 1000) state = STATE_INTRO;
    } else if (state == STATE_WIN) {
        display_clear();
        drawText(35, 30, "VICTORY!");
        display_render();
        if (fire && now - stateTimer > 1000) state = STATE_INTRO;
    }
}

}
