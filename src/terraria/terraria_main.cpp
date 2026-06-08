#include "terraria_main.h"
#include "terraria_display.h"
#include "terraria_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Terraria {

const int SCREEN_W = 128;
const int SCREEN_H = 64;
const int BLOCK_SIZE = 4;
const int WORLD_W = 64;
const int WORLD_H = 64;

uint8_t world[WORLD_H][WORLD_W]; // 0=Air, 1=Dirt, 2=Stone, 3=Wood, 4=Leaves

int inv[5] = {0,0,0,0,0};

float pX = 32.0f * BLOCK_SIZE;
float pY = 10.0f * BLOCK_SIZE;
float vX = 0;
float vY = 0;
bool onGround = false;

int camX = 0;
int camY = 0;

int hp = 5;

bool cursorMode = false;
int curX = 0;
int curY = 0;
unsigned long cursorTimer = 0;

struct Slime {
    float x, y;
    float vx, vy;
    bool active;
};

const int MAX_SLIMES = 3;
Slime slimes[MAX_SLIMES];

void generateWorld() {
    // Fill air
    for(int y=0; y<WORLD_H; y++) {
        for(int x=0; x<WORLD_W; x++) {
            world[y][x] = 0;
        }
    }
    
    // Surface height
    int h = 20;
    for(int x=0; x<WORLD_W; x++) {
        h += random(-1, 2);
        if (h < 10) h = 10;
        if (h > 40) h = 40;
        
        // Fill column
        for(int y=0; y<WORLD_H; y++) {
            if (y > h) {
                if (y > h + 10) {
                    // Deep underground = stone
                    if (random(0, 100) < 80) world[y][x] = 2;
                    else world[y][x] = 1; // patches of dirt
                } else {
                    // Dirt
                    if (random(0, 100) < 90) world[y][x] = 1;
                    else world[y][x] = 2; // patches of stone
                }
            }
        }
        
        // Trees
        if (x > 2 && x < WORLD_W-2 && random(0, 100) < 15) {
            // Check if flat
            if (world[h][x] == 0 && world[h+1][x] == 1) {
                int treeH = random(3, 6);
                for(int t=0; t<treeH; t++) {
                    world[h-t][x] = 3; // Wood
                }
                // Leaves
                world[h-treeH][x] = 4;
                world[h-treeH][x-1] = 4;
                world[h-treeH][x+1] = 4;
                world[h-treeH-1][x] = 4;
            }
        }
        
        // Spawn point
        if (x == 32) {
            pX = x * BLOCK_SIZE;
            pY = (h - 2) * BLOCK_SIZE;
        }
    }
    
    // Clear inventory
    for(int i=0; i<5; i++) inv[i] = 0;
    
    // Clear slimes
    for(int i=0; i<MAX_SLIMES; i++) slimes[i].active = false;
    
    hp = 5;
    cursorMode = false;
}

void setup() {
    display_setup();
    input_setup();
    generateWorld();
}

bool checkCollision(float x, float y, float w, float h) {
    int tx1 = x / BLOCK_SIZE;
    int ty1 = y / BLOCK_SIZE;
    int tx2 = (x + w - 0.1f) / BLOCK_SIZE;
    int ty2 = (y + h - 0.1f) / BLOCK_SIZE;
    
    if (tx1 < 0) tx1 = 0;
    if (tx2 >= WORLD_W) tx2 = WORLD_W-1;
    if (ty1 < 0) ty1 = 0;
    if (ty2 >= WORLD_H) ty2 = WORLD_H-1;
    
    for(int ty=ty1; ty<=ty2; ty++) {
        for(int tx=tx1; tx<=tx2; tx++) {
            if (world[ty][tx] != 0) return true;
        }
    }
    return false;
}

unsigned long lastTime = 0;
unsigned long lastSlimeSpawn = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    if (hp <= 0) {
        display_clear();
        drawText(35, 20, "YOU DIED!");
        drawText(15, 40, "PRESS TO RESPAWN");
        display_render();
        if (input_action()) {
            generateWorld();
        }
        return;
    }
    
    bool action = input_action();
    
    // Toggle cursor mode
    if (input_down() && action && now - cursorTimer > 500) {
        cursorMode = !cursorMode;
        cursorTimer = now;
        curX = (pX + 2) / BLOCK_SIZE;
        curY = (pY + 4) / BLOCK_SIZE;
        tone(BUZZER_PIN, 1000, 50);
        return;
    }
    
    if (cursorMode) {
        // --- Cursor Mode Logic ---
        if (now - cursorTimer > 150) {
            if (input_left()) { curX--; cursorTimer = now; }
            if (input_right()) { curX++; cursorTimer = now; }
            if (input_up()) { curY--; cursorTimer = now; }
            if (input_down()) { curY++; cursorTimer = now; }
        }
        
        // Clamp cursor distance (radius 4 blocks)
        int pbx = (pX+2) / BLOCK_SIZE;
        int pby = (pY+4) / BLOCK_SIZE;
        if (curX < pbx - 4) curX = pbx - 4;
        if (curX > pbx + 4) curX = pbx + 4;
        if (curY < pby - 4) curY = pby - 4;
        if (curY > pby + 4) curY = pby + 4;
        
        if (curX < 0) curX = 0; if (curX >= WORLD_W) curX = WORLD_W-1;
        if (curY < 0) curY = 0; if (curY >= WORLD_H) curY = WORLD_H-1;
        
        if (action && now - cursorTimer > 200) {
            cursorTimer = now;
            int targetBlock = world[curY][curX];
            
            if (targetBlock != 0) {
                // Mine
                inv[targetBlock]++;
                world[curY][curX] = 0;
                tone(BUZZER_PIN, 600, 50);
            } else {
                // Place
                // Find first item in inv > 0
                int placeType = 0;
                for(int i=1; i<5; i++) {
                    if (inv[i] > 0) { placeType = i; break; }
                }
                if (placeType > 0) {
                    // Check if player is not in the way
                    if (!checkCollision(curX*BLOCK_SIZE, curY*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE)) {
                        world[curY][curX] = placeType;
                        inv[placeType]--;
                        tone(BUZZER_PIN, 800, 50);
                    }
                }
            }
        }
        
    } else {
        // --- Normal Gameplay ---
        
        // Input
        if (input_left()) vX = -30.0f;
        else if (input_right()) vX = 30.0f;
        else vX = 0;
        
        if (input_up() && onGround) {
            vY = -80.0f; // Jump
            onGround = false;
            tone(BUZZER_PIN, 400, 30);
        }
        
        // Physics
        vY += 200.0f * dt; // Gravity
        if (vY > 150.0f) vY = 150.0f;
        
        // X Movement
        pX += vX * dt;
        if (checkCollision(pX, pY, 4, 8)) {
            pX -= vX * dt;
        }
        // Bounds
        if (pX < 0) pX = 0;
        if (pX > WORLD_W * BLOCK_SIZE - 4) pX = WORLD_W * BLOCK_SIZE - 4;
        
        // Y Movement
        pY += vY * dt;
        onGround = false;
        if (checkCollision(pX, pY, 4, 8)) {
            pY -= vY * dt;
            if (vY > 0) onGround = true;
            vY = 0;
        }
        
        // Slime Spawn
        if (now - lastSlimeSpawn > 3000) {
            lastSlimeSpawn = now;
            for(int i=0; i<MAX_SLIMES; i++) {
                if (!slimes[i].active) {
                    // Spawn offscreen top
                    slimes[i].active = true;
                    slimes[i].x = pX + (random(0,2)==0 ? -50 : 50);
                    slimes[i].y = pY - 40;
                    slimes[i].vx = 0;
                    slimes[i].vy = 0;
                    break;
                }
            }
        }
        
        // Slime update
        for(int i=0; i<MAX_SLIMES; i++) {
            if (slimes[i].active) {
                // Gravity
                slimes[i].vy += 200.0f * dt;
                
                // AI Hop
                if (slimes[i].vy == 0 && random(0, 100) < 5) {
                    slimes[i].vy = -60.0f;
                    slimes[i].vx = (pX > slimes[i].x) ? 20.0f : -20.0f;
                }
                
                slimes[i].x += slimes[i].vx * dt;
                if (checkCollision(slimes[i].x, slimes[i].y, 4, 4)) {
                    slimes[i].x -= slimes[i].vx * dt;
                    slimes[i].vx = -slimes[i].vx; // Bounce wall
                }
                
                slimes[i].y += slimes[i].vy * dt;
                if (checkCollision(slimes[i].x, slimes[i].y, 4, 4)) {
                    slimes[i].y -= slimes[i].vy * dt;
                    slimes[i].vy = 0;
                    slimes[i].vx = 0;
                }
                
                // Attack Player
                if (action && abs(pX - slimes[i].x) < 12 && abs(pY - slimes[i].y) < 12) {
                    slimes[i].active = false;
                    tone(BUZZER_PIN, 2000, 50); // Kill slime
                } else if (abs(pX - slimes[i].x) < 6 && abs(pY - slimes[i].y) < 6) {
                    // Hurt player
                    if (now % 100 < 50) {
                        hp--;
                        slimes[i].vx = (pX > slimes[i].x) ? -40.0f : 40.0f; // bounce away
                        tone(BUZZER_PIN, 100, 200);
                    }
                }
            }
        }
    }
    
    // Camera follow
    camX = pX - SCREEN_W / 2;
    camY = pY - SCREEN_H / 2;
    if (camX < 0) camX = 0;
    if (camX > WORLD_W * BLOCK_SIZE - SCREEN_W) camX = WORLD_W * BLOCK_SIZE - SCREEN_W;
    if (camY < 0) camY = 0;
    if (camY > WORLD_H * BLOCK_SIZE - SCREEN_H) camY = WORLD_H * BLOCK_SIZE - SCREEN_H;
    
    // --- Rendering ---
    display_clear();
    
    // Draw tiles
    int startX = camX / BLOCK_SIZE;
    int endX = (camX + SCREEN_W) / BLOCK_SIZE + 1;
    int startY = camY / BLOCK_SIZE;
    int endY = (camY + SCREEN_H) / BLOCK_SIZE + 1;
    
    if (startX < 0) startX = 0; if (endX > WORLD_W) endX = WORLD_W;
    if (startY < 0) startY = 0; if (endY > WORLD_H) endY = WORLD_H;
    
    for(int y=startY; y<endY; y++) {
        for(int x=startX; x<endX; x++) {
            if (world[y][x] != 0) {
                drawBlock(x * BLOCK_SIZE - camX, y * BLOCK_SIZE - camY, world[y][x]);
            }
        }
    }
    
    // Draw player
    int drawPx = (int)pX - camX;
    int drawPy = (int)pY - camY;
    fillRect(drawPx, drawPy, 4, 8, 1);
    drawPixel(drawPx + 1, drawPy + 2, 0); // Eye
    
    // Draw Slimes
    for(int i=0; i<MAX_SLIMES; i++) {
        if (slimes[i].active) {
            int sx = (int)slimes[i].x - camX;
            int sy = (int)slimes[i].y - camY;
            drawRect(sx, sy, 4, 4, 1);
            drawPixel(sx+1, sy+1, 1);
        }
    }
    
    // Draw Cursor
    if (cursorMode) {
        int cx = curX * BLOCK_SIZE - camX;
        int cy = curY * BLOCK_SIZE - camY;
        drawLine(cx, cy, cx+2, cy, 1);
        drawLine(cx, cy, cx, cy+2, 1);
        drawLine(cx+3, cy+3, cx+1, cy+3, 1);
        drawLine(cx+3, cy+3, cx+3, cy+1, 1);
        
        // Show current inv item
        int placeType = 0;
        for(int i=1; i<5; i++) {
            if (inv[i] > 0) { placeType = i; break; }
        }
        if (placeType > 0) {
            drawBlock(cx, cy, placeType);
        }
        
        drawText(2, 2, "CURSOR MODE");
    }
    
    // HUD
    for(int i=0; i<hp; i++) {
        drawRect(2 + i*5, 58, 4, 4, 1);
    }
    
    // Show total inventory
    int totalInv = inv[1]+inv[2]+inv[3]+inv[4];
    drawText(100, 56, "B:");
    drawText(112, 56, totalInv);
    
    display_render();
}

}
