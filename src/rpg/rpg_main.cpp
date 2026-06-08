#include "rpg_main.h"
#include "rpg_display.h"
#include "rpg_input.h"

#define BUZZER_PIN 14

namespace RPG {

const int SCREEN_W = 128;
const int SCREEN_H = 64;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_WIN
};

GameState state = STATE_INTRO;

// --- Map ---
// 3x3 grid of rooms.
int roomX = 1;
int roomY = 1;

// --- Player ---
float px = 64.0f;
float py = 32.0f;
float pDirX = 0.0f;
float pDirY = 1.0f;
int hp = 5;
bool hasKey = false;
unsigned long attackTimer = 0;
bool isAttacking = false;

// --- Enemies ---
const int MAX_ENEMIES = 3;
struct Enemy {
    float x, y;
    int type; // 1=Slime, 2=Bat
    int hp;
    bool active;
} enemies[MAX_ENEMIES];

// --- Room Data ---
// 0=Empty, 1=Wall, 2=Key, 3=Door
uint8_t roomData[9][8][16]; // 3x3 rooms, 16x8 tiles (8x8 pixels)

void initMap() {
    // Fill with walls borders
    for(int ry=0; ry<3; ry++) {
        for(int rx=0; rx<3; rx++) {
            for(int y=0; y<8; y++) {
                for(int x=0; x<16; x++) {
                    if (y == 0 || y == 7 || x == 0 || x == 15) {
                        // Leave holes for doors
                        if ((x==7 || x==8) && y==0 && ry>0) roomData[ry*3+rx][y][x] = 0; // Top door
                        else if ((x==7 || x==8) && y==7 && ry<2) roomData[ry*3+rx][y][x] = 0; // Bottom door
                        else if ((y==3 || y==4) && x==0 && rx>0) roomData[ry*3+rx][y][x] = 0; // Left door
                        else if ((y==3 || y==4) && x==15 && rx<2) roomData[ry*3+rx][y][x] = 0; // Right door
                        else roomData[ry*3+rx][y][x] = 1;
                    } else {
                        roomData[ry*3+rx][y][x] = 0;
                        // Random obstacles
                        if (random(0, 100) < 5) roomData[ry*3+rx][y][x] = 1;
                    }
                }
            }
        }
    }
    
    // Key in top-left (0,0)
    roomData[0][3][7] = 2;
    // Door in top-right (2,0)
    roomData[2][3][7] = 3;
}

void loadRoom() {
    // Clear enemies
    for(int i=0; i<MAX_ENEMIES; i++) enemies[i].active = false;
    
    // Spawn enemies if not start room
    if (roomX != 1 || roomY != 1) {
        int count = random(1, 4);
        for(int i=0; i<count; i++) {
            enemies[i].active = true;
            enemies[i].x = random(20, 108);
            enemies[i].y = random(20, 44);
            enemies[i].type = random(1, 3);
            enemies[i].hp = enemies[i].type == 1 ? 2 : 1;
        }
    }
}

void resetGame() {
    hp = 5;
    hasKey = false;
    roomX = 1;
    roomY = 1;
    px = 64.0f;
    py = 32.0f;
    pDirX = 0; pDirY = 1;
    initMap();
    loadRoom();
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
    
    bool fire = input_attack();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(15, 20, "DUNGEON EXPLORER");
        drawText(20, 40, "PRESS TO START");
        display_render();
        
        if (fire) {
            resetGame();
            state = STATE_PLAYING;
        }
    } else if (state == STATE_PLAYING) {
        
        // --- Input ---
        float ix = input_x();
        float iy = input_y();
        
        if (!isAttacking) {
            if (abs(ix) > 0.1f || abs(iy) > 0.1f) {
                // Normalize for direction
                float len = sqrt(ix*ix + iy*iy);
                pDirX = ix / len;
                pDirY = iy / len;
                
                float nx = px + ix * 50.0f * dt;
                float ny = py + iy * 50.0f * dt;
                
                // Tile collision
                int tx = (int)nx / 8;
                int ty = (int)ny / 8;
                if (tx >= 0 && tx < 16 && ty >= 0 && ty < 8) {
                    uint8_t tile = roomData[roomY*3+roomX][ty][tx];
                    if (tile == 0 || tile == 2) {
                        px = nx;
                        py = ny;
                    } else if (tile == 3 && hasKey) {
                        state = STATE_WIN;
                        tone(BUZZER_PIN, 1500, 500);
                    }
                }
            }
            
            // Collectables
            int tx = (int)px / 8;
            int ty = (int)py / 8;
            if (roomData[roomY*3+roomX][ty][tx] == 2) {
                hasKey = true;
                roomData[roomY*3+roomX][ty][tx] = 0;
                tone(BUZZER_PIN, 1200, 200);
            }
            
            if (fire && now - attackTimer > 500) {
                isAttacking = true;
                attackTimer = now;
                tone(BUZZER_PIN, 600, 50);
            }
        }
        
        if (isAttacking && now - attackTimer > 150) {
            isAttacking = false;
        }
        
        // --- Room Transitions ---
        if (px < 0) { if (roomX > 0) roomX--; px = SCREEN_W - 4; loadRoom(); }
        else if (px > SCREEN_W) { if (roomX < 2) roomX++; px = 4; loadRoom(); }
        else if (py < 0) { if (roomY > 0) roomY--; py = SCREEN_H - 4; loadRoom(); }
        else if (py > SCREEN_H) { if (roomY < 2) roomY++; py = 4; loadRoom(); }
        
        // --- Enemies ---
        for (int i=0; i<MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                if (enemies[i].type == 1) { // Slime (slow follow)
                    float dx = px - enemies[i].x;
                    float dy = py - enemies[i].y;
                    float dist = sqrt(dx*dx + dy*dy);
                    if (dist > 0) {
                        enemies[i].x += (dx/dist) * 15.0f * dt;
                        enemies[i].y += (dy/dist) * 15.0f * dt;
                    }
                } else if (enemies[i].type == 2) { // Bat (erratic)
                    enemies[i].x += random(-30, 31) * dt;
                    enemies[i].y += random(-30, 31) * dt;
                }
                
                // Damage player
                if (abs(px - enemies[i].x) < 6 && abs(py - enemies[i].y) < 6) {
                    hp--;
                    // Knockback
                    px += (px - enemies[i].x) > 0 ? 10 : -10;
                    py += (py - enemies[i].y) > 0 ? 10 : -10;
                    tone(BUZZER_PIN, 100, 200);
                    if (hp <= 0) state = STATE_GAMEOVER;
                }
                
                // Damage enemy
                if (isAttacking) {
                    float ax = px + pDirX * 12.0f;
                    float ay = py + pDirY * 12.0f;
                    if (abs(ax - enemies[i].x) < 10 && abs(ay - enemies[i].y) < 10) {
                        enemies[i].hp--;
                        enemies[i].x += pDirX * 10;
                        enemies[i].y += pDirY * 10;
                        if (enemies[i].hp <= 0) {
                            enemies[i].active = false;
                            tone(BUZZER_PIN, 800, 50);
                        }
                    }
                }
            }
        }
        
        // --- Render ---
        display_clear();
        
        // Map
        for(int y=0; y<8; y++) {
            for(int x=0; x<16; x++) {
                uint8_t t = roomData[roomY*3+roomX][y][x];
                if (t == 1) drawRect(x*8, y*8, 8, 8, 1);
                else if (t == 2) { // Key
                    drawCircle(x*8+4, y*8+3, 2, 1);
                    drawLine(x*8+4, y*8+5, x*8+4, y*8+8, 1);
                    drawLine(x*8+4, y*8+7, x*8+6, y*8+7, 1);
                } else if (t == 3) { // Door
                    drawRect(x*8+1, y*8+1, 6, 6, 1);
                    if (!hasKey) drawLine(x*8+1, y*8+1, x*8+7, y*8+7, 1); // Locked
                }
            }
        }
        
        // Enemies
        for (int i=0; i<MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                if (enemies[i].type == 1) { // Slime
                    fillRect((int)enemies[i].x - 3, (int)enemies[i].y - 2, 6, 4, 1);
                } else { // Bat
                    drawLine((int)enemies[i].x-4, (int)enemies[i].y-2, (int)enemies[i].x+4, (int)enemies[i].y-2, 1);
                    drawPixel((int)enemies[i].x, (int)enemies[i].y, 1);
                }
            }
        }
        
        // Player
        drawCircle((int)px, (int)py, 3, 1);
        drawLine((int)px, (int)py, (int)(px + pDirX*5), (int)(py + pDirY*5), 1);
        
        // Attack Arc
        if (isAttacking) {
            int cx = (int)(px + pDirX * 10);
            int cy = (int)(py + pDirY * 10);
            drawCircle(cx, cy, 4, 1);
        }
        
        // HUD
        for(int i=0; i<hp; i++) {
            fillRect(2 + (i*4), 2, 3, 3, 1);
        }
        if (hasKey) {
            drawText(100, 2, "KEY");
        }
        
        display_render();
        
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(35, 30, "YOU DIED");
        display_render();
        if (fire) state = STATE_INTRO;
    } else if (state == STATE_WIN) {
        display_clear();
        drawText(35, 30, "VICTORY!");
        display_render();
        if (fire) state = STATE_INTRO;
    }
}

}
