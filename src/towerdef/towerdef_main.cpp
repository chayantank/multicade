#include "towerdef_main.h"
#include "towerdef_display.h"
#include "towerdef_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Towerdef {

struct Point {
    float x, y;
};

const int NUM_POINTS = 6;
Point path[NUM_POINTS] = {
    {-10, 10}, {40, 10}, {40, 40}, {90, 40}, {90, 20}, {138, 20}
};

struct Enemy {
    float x, y;
    int targetNode;
    float hp;
    bool active;
};

const int MAX_ENEMIES = 10;
Enemy enemies[MAX_ENEMIES];

struct Turret {
    float x, y;
    float cooldown;
    bool active;
};

const int MAX_TURRETS = 10;
Turret turrets[MAX_TURRETS];

struct Bullet {
    float x, y;
    float vx, vy;
    bool active;
};

const int MAX_BULLETS = 15;
Bullet bullets[MAX_BULLETS];

float cursorX = 64;
float cursorY = 32;

int money = 50;
int lives = 10;
int wave = 1;
bool gameOver = false;

unsigned long lastSpawn = 0;
int enemiesToSpawn = 0;

void resetGame() {
    money = 50;
    lives = 10;
    wave = 1;
    gameOver = false;
    enemiesToSpawn = 5;
    for(int i=0; i<MAX_ENEMIES; i++) enemies[i].active = false;
    for(int i=0; i<MAX_TURRETS; i++) turrets[i].active = false;
    for(int i=0; i<MAX_BULLETS; i++) bullets[i].active = false;
    cursorX = 64;
    cursorY = 32;
}

void setup() {
    display_setup();
    input_setup();
    resetGame();
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
        drawText(35, 30, "WAVE:");
        drawText(70, 30, wave);
        display_render();
        if (input_action()) {
            tone(BUZZER_PIN, 800, 100); delay(200);
            resetGame();
        }
        return;
    }
    
    if (enemiesToSpawn > 0 && now - lastSpawn > 1000) {
        for(int i=0; i<MAX_ENEMIES; i++) {
            if (!enemies[i].active) {
                enemies[i].active = true;
                enemies[i].x = path[0].x;
                enemies[i].y = path[0].y;
                enemies[i].targetNode = 1;
                enemies[i].hp = 10 + (wave * 5);
                lastSpawn = now;
                enemiesToSpawn--;
                break;
            }
        }
    }
    
    bool waveActive = (enemiesToSpawn > 0);
    for(int i=0; i<MAX_ENEMIES; i++) if(enemies[i].active) waveActive = true;
    if (!waveActive) {
        wave++;
        enemiesToSpawn = 5 + (wave * 2);
        lastSpawn = now + 2000;
    }
    
    int jx = input_x() - 2048;
    int jy = input_y() - 2048;
    
    if (abs(jx) > 500) cursorX += (jx > 0 ? 1 : -1) * 80.0f * dt;
    if (abs(jy) > 500) cursorY += (jy > 0 ? 1 : -1) * 80.0f * dt;
    if (cursorX < 0) cursorX = 0; if (cursorX > 127) cursorX = 127;
    if (cursorY < 0) cursorY = 0; if (cursorY > 63) cursorY = 63;
    
    if (input_action() && money >= 20) {
        int bx = ((int)cursorX / 10) * 10 + 5;
        int by = ((int)cursorY / 10) * 10 + 5;
        
        bool canBuild = true;
        for(int i=0; i<MAX_TURRETS; i++) {
            if (turrets[i].active && abs(turrets[i].x - bx) < 5 && abs(turrets[i].y - by) < 5) canBuild = false;
        }
        for(int i=0; i<NUM_POINTS-1; i++) {
            float minX = min(path[i].x, path[i+1].x) - 5;
            float maxX = max(path[i].x, path[i+1].x) + 5;
            float minY = min(path[i].y, path[i+1].y) - 5;
            float maxY = max(path[i].y, path[i+1].y) + 5;
            if (bx > minX && bx < maxX && by > minY && by < maxY) canBuild = false;
        }
        
        if (canBuild) {
            for(int i=0; i<MAX_TURRETS; i++) {
                if (!turrets[i].active) {
                    turrets[i].active = true;
                    turrets[i].x = bx;
                    turrets[i].y = by;
                    turrets[i].cooldown = 0;
                    money -= 20;
                    tone(BUZZER_PIN, 1500, 50);
                    break;
                }
            }
        }
    }
    
    for(int i=0; i<MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            Point target = path[enemies[i].targetNode];
            float dx = target.x - enemies[i].x;
            float dy = target.y - enemies[i].y;
            float dist = sqrt(dx*dx + dy*dy);
            
            if (dist < 2.0f) {
                enemies[i].targetNode++;
                if (enemies[i].targetNode >= NUM_POINTS) {
                    enemies[i].active = false;
                    lives--;
                    tone(BUZZER_PIN, 100, 300);
                    if (lives <= 0) gameOver = true;
                }
            } else {
                float speed = 20.0f;
                enemies[i].x += (dx/dist) * speed * dt;
                enemies[i].y += (dy/dist) * speed * dt;
            }
        }
    }
    
    for(int i=0; i<MAX_TURRETS; i++) {
        if (turrets[i].active) {
            turrets[i].cooldown -= dt;
            if (turrets[i].cooldown <= 0) {
                for(int e=0; e<MAX_ENEMIES; e++) {
                    if (enemies[e].active) {
                        float dx = enemies[e].x - turrets[i].x;
                        float dy = enemies[e].y - turrets[i].y;
                        float dist = sqrt(dx*dx + dy*dy);
                        if (dist < 30.0f) {
                            for(int b=0; b<MAX_BULLETS; b++) {
                                if (!bullets[b].active) {
                                    bullets[b].active = true;
                                    bullets[b].x = turrets[i].x;
                                    bullets[b].y = turrets[i].y;
                                    bullets[b].vx = (dx/dist) * 100.0f;
                                    bullets[b].vy = (dy/dist) * 100.0f;
                                    turrets[i].cooldown = 0.5f;
                                    tone(BUZZER_PIN, 2000, 10);
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    
    for(int b=0; b<MAX_BULLETS; b++) {
        if (bullets[b].active) {
            bullets[b].x += bullets[b].vx * dt;
            bullets[b].y += bullets[b].vy * dt;
            if (bullets[b].x < 0 || bullets[b].x > 128 || bullets[b].y < 0 || bullets[b].y > 64) {
                bullets[b].active = false;
            } else {
                for(int e=0; e<MAX_ENEMIES; e++) {
                    if (enemies[e].active) {
                        if (abs(enemies[e].x - bullets[b].x) < 4 && abs(enemies[e].y - bullets[b].y) < 4) {
                            enemies[e].hp -= 10;
                            bullets[b].active = false;
                            if (enemies[e].hp <= 0) {
                                enemies[e].active = false;
                                money += 5;
                                tone(BUZZER_PIN, 500, 30);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    
    display_clear();
    
    for(int i=0; i<NUM_POINTS-1; i++) {
        drawLine(path[i].x, path[i].y, path[i+1].x, path[i+1].y, 1);
        drawLine(path[i].x, path[i].y+1, path[i+1].x, path[i+1].y+1, 1);
    }
    
    drawRect(120, 15, 8, 10, 1);
    
    for(int i=0; i<MAX_TURRETS; i++) {
        if (turrets[i].active) {
            drawRect((int)turrets[i].x-2, (int)turrets[i].y-2, 4, 4, 1);
        }
    }
    
    for(int i=0; i<MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            drawCircle((int)enemies[i].x, (int)enemies[i].y, 2, 1);
        }
    }
    
    for(int i=0; i<MAX_BULLETS; i++) {
        if (bullets[i].active) {
            drawLine((int)bullets[i].x, (int)bullets[i].y, (int)(bullets[i].x-bullets[i].vx*0.02f), (int)(bullets[i].y-bullets[i].vy*0.02f), 1);
        }
    }
    
    drawLine((int)cursorX-2, (int)cursorY, (int)cursorX+2, (int)cursorY, 1);
    drawLine((int)cursorX, (int)cursorY-2, (int)cursorX, (int)cursorY+2, 1);
    
    drawText(2, 54, "$"); drawText(10, 54, money);
    drawText(40, 54, "L:"); drawText(52, 54, lives);
    drawText(80, 54, "W:"); drawText(92, 54, wave);
    
    display_render();
}

}
