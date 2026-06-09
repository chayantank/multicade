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

// 0=Air, 1=Dirt, 2=Stone, 3=Wood, 4=Leaves, 5=Gold, 6=Diamond
uint8_t world[WORLD_H][WORLD_W]; 

int inv[5] = {0,0,0,0,0};
int score = 0;

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

float timeOfDay = 0; // 0 to 60 seconds cycle (30s day, 30s night)

enum EnemyType { SLIME, ZOMBIE };

struct Enemy {
    float x, y;
    float vx, vy;
    EnemyType type;
    bool active;
};

const int MAX_ENEMIES = 3;
Enemy enemies[MAX_ENEMIES];

void generateWorld() {
    for(int y=0; y<WORLD_H; y++) {
        for(int x=0; x<WORLD_W; x++) {
            world[y][x] = 0;
        }
    }
    
    int h = 20;
    for(int x=0; x<WORLD_W; x++) {
        h += random(-1, 2);
        if (h < 10) h = 10;
        if (h > 40) h = 40;
        
        for(int y=0; y<WORLD_H; y++) {
            if (y > h) {
                if (y > h + 15) {
                    if (random(0, 100) < 5) world[y][x] = 6; // Diamond
                    else if (random(0, 100) < 15) world[y][x] = 5; // Gold
                    else if (random(0, 100) < 80) world[y][x] = 2; // Stone
                    else world[y][x] = 1; // patches of dirt
                } else if (y > h + 5) {
                    if (random(0, 100) < 5) world[y][x] = 5; // Gold
                    else if (random(0, 100) < 60) world[y][x] = 2;
                    else world[y][x] = 1;
                } else {
                    if (random(0, 100) < 90) world[y][x] = 1; // Dirt
                    else world[y][x] = 2; // patches of stone
                }
            }
        }
        
        if (x > 2 && x < WORLD_W-2 && random(0, 100) < 15) {
            if (world[h][x] == 0 && world[h+1][x] == 1) {
                int treeH = random(3, 6);
                for(int t=0; t<treeH; t++) {
                    world[h-t][x] = 3; // Wood
                }
                world[h-treeH][x] = 4;
                world[h-treeH][x-1] = 4;
                world[h-treeH][x+1] = 4;
                world[h-treeH-1][x] = 4;
            }
        }
        
        if (x == 32) {
            pX = x * BLOCK_SIZE;
            pY = (h - 2) * BLOCK_SIZE;
        }
    }
    
    for(int i=0; i<5; i++) inv[i] = 0;
    for(int i=0; i<MAX_ENEMIES; i++) enemies[i].active = false;
    
    hp = 5;
    score = 0;
    timeOfDay = 0;
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
        drawText(30, 40, "SCORE:");
        drawText(70, 40, score);
        display_render();
        if (input_action() && now % 500 < 50) generateWorld();
        return;
    }
    
    timeOfDay += dt;
    if (timeOfDay > 60.0f) timeOfDay = 0;
    bool isNight = (timeOfDay > 30.0f);
    
    bool action = input_action();
    
    if (input_down() && action && now - cursorTimer > 500) {
        cursorMode = !cursorMode;
        cursorTimer = now;
        curX = (pX + 2) / BLOCK_SIZE;
        curY = (pY + 4) / BLOCK_SIZE;
        tone(BUZZER_PIN, 1000, 50);
        return;
    }
    
    if (cursorMode) {
        if (now - cursorTimer > 150) {
            if (input_left()) { curX--; cursorTimer = now; }
            if (input_right()) { curX++; cursorTimer = now; }
            if (input_up()) { curY--; cursorTimer = now; }
            if (input_down()) { curY++; cursorTimer = now; }
        }
        
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
                if (targetBlock == 5) { score += 10; tone(BUZZER_PIN, 1500, 100); }
                else if (targetBlock == 6) { score += 50; tone(BUZZER_PIN, 2000, 200); }
                else if (targetBlock < 5) inv[targetBlock]++;
                
                world[curY][curX] = 0;
                if (targetBlock < 5) tone(BUZZER_PIN, 600, 50);
            } else {
                int placeType = 0;
                for(int i=1; i<5; i++) {
                    if (inv[i] > 0) { placeType = i; break; }
                }
                if (placeType > 0) {
                    if (!checkCollision(curX*BLOCK_SIZE, curY*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE)) {
                        world[curY][curX] = placeType;
                        inv[placeType]--;
                        tone(BUZZER_PIN, 800, 50);
                    }
                }
            }
        }
    } else {
        if (input_left()) vX = -30.0f;
        else if (input_right()) vX = 30.0f;
        else vX = 0;
        
        if (input_up() && onGround) {
            vY = -80.0f; 
            onGround = false;
            tone(BUZZER_PIN, 400, 30);
        }
        
        vY += 200.0f * dt; 
        if (vY > 150.0f) vY = 150.0f;
        
        pX += vX * dt;
        if (checkCollision(pX, pY, 4, 8)) pX -= vX * dt;
        
        if (pX < 0) pX = 0;
        if (pX > WORLD_W * BLOCK_SIZE - 4) pX = WORLD_W * BLOCK_SIZE - 4;
        
        pY += vY * dt;
        onGround = false;
        if (checkCollision(pX, pY, 4, 8)) {
            pY -= vY * dt;
            if (vY > 0) onGround = true;
            vY = 0;
        }
        
        int spawnRate = isNight ? 2000 : 4000;
        if (now - lastSlimeSpawn > spawnRate) {
            lastSlimeSpawn = now;
            for(int i=0; i<MAX_ENEMIES; i++) {
                if (!enemies[i].active) {
                    enemies[i].active = true;
                    enemies[i].x = pX + (random(0,2)==0 ? -60 : 60);
                    enemies[i].y = pY - 40;
                    enemies[i].vx = 0;
                    enemies[i].vy = 0;
                    enemies[i].type = isNight ? ZOMBIE : SLIME;
                    break;
                }
            }
        }
        
        for(int i=0; i<MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                enemies[i].vy += 200.0f * dt;
                
                if (enemies[i].type == SLIME) {
                    if (enemies[i].vy == 0 && random(0, 100) < 5) {
                        enemies[i].vy = -60.0f;
                        enemies[i].vx = (pX > enemies[i].x) ? 20.0f : -20.0f;
                    }
                } else if (enemies[i].type == ZOMBIE) {
                    if (enemies[i].vy == 0) {
                        enemies[i].vx = (pX > enemies[i].x) ? 15.0f : -15.0f;
                        if (checkCollision(enemies[i].x + (enemies[i].vx > 0 ? 4 : -1), enemies[i].y, 4, 4)) {
                            enemies[i].vy = -70.0f;
                        }
                    } else {
                        enemies[i].vx *= 0.95f;
                    }
                }
                
                enemies[i].x += enemies[i].vx * dt;
                if (checkCollision(enemies[i].x, enemies[i].y, 4, enemies[i].type==ZOMBIE?8:4)) {
                    enemies[i].x -= enemies[i].vx * dt;
                    enemies[i].vx = -enemies[i].vx; 
                }
                
                enemies[i].y += enemies[i].vy * dt;
                if (checkCollision(enemies[i].x, enemies[i].y, 4, enemies[i].type==ZOMBIE?8:4)) {
                    enemies[i].y -= enemies[i].vy * dt;
                    enemies[i].vy = 0;
                    if(enemies[i].type == SLIME) enemies[i].vx = 0;
                }
                
                int eH = (enemies[i].type == ZOMBIE) ? 8 : 4;
                if (action && abs(pX - enemies[i].x) < 12 && abs(pY - enemies[i].y) < 12) {
                    enemies[i].active = false;
                    score += (enemies[i].type == ZOMBIE ? 2 : 1);
                    tone(BUZZER_PIN, 2000, 50); 
                } else if (abs(pX - enemies[i].x) < 6 && abs(pY - enemies[i].y) < (eH+4)/2.0f) {
                    if (now % 200 < 50) {
                        hp -= (enemies[i].type == ZOMBIE ? 2 : 1);
                        enemies[i].vx = (pX > enemies[i].x) ? -40.0f : 40.0f; 
                        tone(BUZZER_PIN, 100, 200);
                    }
                }
            }
        }
    }
    
    camX = pX - SCREEN_W / 2;
    camY = pY - SCREEN_H / 2;
    if (camX < 0) camX = 0;
    if (camX > WORLD_W * BLOCK_SIZE - SCREEN_W) camX = WORLD_W * BLOCK_SIZE - SCREEN_W;
    if (camY < 0) camY = 0;
    if (camY > WORLD_H * BLOCK_SIZE - SCREEN_H) camY = WORLD_H * BLOCK_SIZE - SCREEN_H;
    
    display_clear();
    
    int startX = camX / BLOCK_SIZE;
    int endX = (camX + SCREEN_W) / BLOCK_SIZE + 1;
    int startY = camY / BLOCK_SIZE;
    int endY = (camY + SCREEN_H) / BLOCK_SIZE + 1;
    
    if (startX < 0) startX = 0; if (endX > WORLD_W) endX = WORLD_W;
    if (startY < 0) startY = 0; if (endY > WORLD_H) endY = WORLD_H;
    
    for(int y=startY; y<endY; y++) {
        for(int x=startX; x<endX; x++) {
            if (world[y][x] != 0) {
                int drawX = x * BLOCK_SIZE - camX;
                int drawY = y * BLOCK_SIZE - camY;
                if (world[y][x] < 5) {
                    drawBlock(drawX, drawY, world[y][x]);
                } else if (world[y][x] == 5) { // Gold
                    drawRect(drawX, drawY, BLOCK_SIZE, BLOCK_SIZE, 1);
                    drawPixel(drawX+1, drawY+1, 1); drawPixel(drawX+2, drawY+2, 1);
                } else if (world[y][x] == 6) { // Diamond
                    drawRect(drawX, drawY, BLOCK_SIZE, BLOCK_SIZE, 1);
                    fillRect(drawX+1, drawY+1, 2, 2, 1);
                }
            }
        }
    }
    
    int drawPx = (int)pX - camX;
    int drawPy = (int)pY - camY;
    fillRect(drawPx, drawPy, 4, 8, 1);
    drawPixel(drawPx + (vX<0?1:2), drawPy + 2, 0); 
    
    for(int i=0; i<MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            int sx = (int)enemies[i].x - camX;
            int sy = (int)enemies[i].y - camY;
            if (enemies[i].type == SLIME) {
                drawRect(sx, sy, 4, 4, 1);
                drawPixel(sx+1, sy+1, 1);
            } else {
                fillRect(sx, sy, 4, 8, 1); 
                drawPixel(sx+1, sy+2, 0); drawPixel(sx+2, sy+2, 0); 
            }
        }
    }
    
    if (cursorMode) {
        int cx = curX * BLOCK_SIZE - camX;
        int cy = curY * BLOCK_SIZE - camY;
        drawLine(cx, cy, cx+2, cy, 1); drawLine(cx, cy, cx, cy+2, 1);
        drawLine(cx+3, cy+3, cx+1, cy+3, 1); drawLine(cx+3, cy+3, cx+3, cy+1, 1);
        
        int placeType = 0;
        for(int i=1; i<5; i++) {
            if (inv[i] > 0) { placeType = i; break; }
        }
        if (placeType > 0) drawBlock(cx, cy, placeType);
        
        drawText(2, 2, "CURSOR MODE");
    }
    
    for(int i=0; i<hp; i++) drawRect(2 + i*5, 58, 4, 4, 1);
    
    int totalInv = inv[1]+inv[2]+inv[3]+inv[4];
    drawText(90, 56, "B:"); drawText(102, 56, totalInv);
    
    drawText(2, 10, isNight ? "NIGHT" : "DAY");
    drawText(90, 2, "S:"); drawText(102, 2, score);
    
    display_render();
}

}
