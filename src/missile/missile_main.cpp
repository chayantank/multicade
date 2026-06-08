#include "missile_main.h"
#include "missile_display.h"
#include "missile_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Missile {

const int MAX_CITIES = 3;
bool cities[MAX_CITIES]; // true if alive
int cityX[MAX_CITIES] = {20, 60, 100};

int cursorX = 64;
int cursorY = 32;

struct Expl {
    float x, y;
    float r;
    float maxR;
    bool active;
};

const int MAX_EXPL = 5;
Expl expls[MAX_EXPL];

struct Mis {
    float sx, sy;
    float x, y;
    float vx, vy;
    bool friendly;
    float tx, ty; // target
    bool active;
};

const int MAX_MIS = 10;
Mis miss[MAX_MIS];

int score = 0;
int wave = 1;
int ammo = 10;
int enemyLeft = 5;
bool gameOver = false;

unsigned long lastEnemyTime = 0;

void spawnExplosion(float x, float y, float maxR) {
    for(int i=0; i<MAX_EXPL; i++) {
        if (!expls[i].active) {
            expls[i].active = true;
            expls[i].x = x;
            expls[i].y = y;
            expls[i].r = 1.0f;
            expls[i].maxR = maxR;
            tone(BUZZER_PIN, 200, 100);
            return;
        }
    }
}

void fireFriendly() {
    if (ammo <= 0) {
        tone(BUZZER_PIN, 50, 50); // Empty
        return;
    }
    for(int i=0; i<MAX_MIS; i++) {
        if (!miss[i].active) {
            miss[i].active = true;
            miss[i].friendly = true;
            miss[i].sx = 64; // base
            miss[i].sy = 60;
            miss[i].x = 64;
            miss[i].y = 60;
            miss[i].tx = cursorX;
            miss[i].ty = cursorY;
            
            float dx = miss[i].tx - miss[i].x;
            float dy = miss[i].ty - miss[i].y;
            float mag = sqrt(dx*dx + dy*dy);
            
            miss[i].vx = (dx / mag) * 60.0f;
            miss[i].vy = (dy / mag) * 60.0f;
            
            ammo--;
            tone(BUZZER_PIN, 800, 50);
            return;
        }
    }
}

void fireEnemy() {
    for(int i=0; i<MAX_MIS; i++) {
        if (!miss[i].active) {
            miss[i].active = true;
            miss[i].friendly = false;
            miss[i].sx = random(0, 128);
            miss[i].sy = 0;
            miss[i].x = miss[i].sx;
            miss[i].y = 0;
            
            // Pick a living city
            int targetCity = random(0, 3);
            int tries = 3;
            while(!cities[targetCity] && tries > 0) {
                targetCity = random(0, 3);
                tries--;
            }
            
            miss[i].tx = cityX[targetCity] + 6;
            miss[i].ty = 60;
            
            float dx = miss[i].tx - miss[i].x;
            float dy = miss[i].ty - miss[i].y;
            float mag = sqrt(dx*dx + dy*dy);
            
            float speed = 10.0f + wave * 2.0f;
            
            miss[i].vx = (dx / mag) * speed;
            miss[i].vy = (dy / mag) * speed;
            
            enemyLeft--;
            return;
        }
    }
}

void resetGame() {
    for(int i=0; i<3; i++) cities[i] = true;
    for(int i=0; i<MAX_EXPL; i++) expls[i].active = false;
    for(int i=0; i<MAX_MIS; i++) miss[i].active = false;
    score = 0;
    wave = 1;
    ammo = 15;
    enemyLeft = 5;
    gameOver = false;
    cursorX = 64; cursorY = 32;
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
    if (dt > 0.1f) dt = 0.1f;
    
    if (gameOver) {
        display_clear();
        drawText(35, 20, "GAME OVER");
        drawText(40, 30, score);
        drawText(15, 45, "PRESS TO RESTART");
        display_render();
        if (input_action()) {
            tone(BUZZER_PIN, 800, 100); delay(200);
            resetGame();
        }
        return;
    }
    
    // Wave complete?
    bool waveDone = (enemyLeft <= 0);
    if (waveDone) {
        for(int i=0; i<MAX_MIS; i++) {
            if (miss[i].active && !miss[i].friendly) waveDone = false;
        }
    }
    
    if (waveDone) {
        wave++;
        ammo = 15 + wave*2;
        enemyLeft = 5 + wave*2;
        score += 100;
        tone(BUZZER_PIN, 1000, 200); delay(250); tone(BUZZER_PIN, 1500, 300);
    }
    
    // Check death
    bool allDead = true;
    for(int i=0; i<3; i++) if (cities[i]) allDead = false;
    if (allDead) {
        gameOver = true;
    }
    
    // Cursor input
    int joyX = input_x() - 2048;
    int joyY = input_y() - 2048;
    
    if (abs(joyX) > 500) cursorX += (joyX > 0 ? 1 : -1) * 80.0f * dt;
    if (abs(joyY) > 500) cursorY += (joyY > 0 ? 1 : -1) * 80.0f * dt;
    
    if (cursorX < 0) cursorX = 0; if (cursorX > 127) cursorX = 127;
    if (cursorY < 0) cursorY = 0; if (cursorY > 58) cursorY = 58;
    
    if (input_action()) {
        fireFriendly();
    }
    
    // Enemy Spawn
    if (enemyLeft > 0 && now - lastEnemyTime > (2000.0f / (1.0f + wave*0.2f))) {
        fireEnemy();
        lastEnemyTime = now;
    }
    
    // Update Missiles
    for(int i=0; i<MAX_MIS; i++) {
        if (miss[i].active) {
            miss[i].x += miss[i].vx * dt;
            miss[i].y += miss[i].vy * dt;
            
            if (miss[i].friendly) {
                // Check arrival
                float dist = abs(miss[i].x - miss[i].tx) + abs(miss[i].y - miss[i].ty);
                if (dist < 2.0f) {
                    miss[i].active = false;
                    spawnExplosion(miss[i].tx, miss[i].ty, 12.0f);
                }
            } else {
                // Enemy check ground
                if (miss[i].y >= 60) {
                    miss[i].active = false;
                    spawnExplosion(miss[i].x, miss[i].y, 10.0f);
                    
                    // Kill city
                    for(int c=0; c<3; c++) {
                        if (cities[c] && abs(miss[i].x - (cityX[c]+6)) < 10) {
                            cities[c] = false;
                            tone(BUZZER_PIN, 50, 500); // Bad boom
                        }
                    }
                }
                
                // Enemy check explosion
                for(int e=0; e<MAX_EXPL; e++) {
                    if (expls[e].active) {
                        float dx = miss[i].x - expls[e].x;
                        float dy = miss[i].y - expls[e].y;
                        if (dx*dx + dy*dy <= expls[e].r * expls[e].r) {
                            miss[i].active = false;
                            score += 10 * wave;
                            spawnExplosion(miss[i].x, miss[i].y, 8.0f); // Chain reaction!
                            break;
                        }
                    }
                }
            }
        }
    }
    
    // Update Explosions
    for(int i=0; i<MAX_EXPL; i++) {
        if (expls[i].active) {
            expls[i].r += 15.0f * dt;
            if (expls[i].r >= expls[i].maxR) {
                expls[i].active = false;
            }
        }
    }
    
    // Render
    display_clear();
    
    // Cities
    for(int i=0; i<3; i++) {
        if (cities[i]) drawCity(cityX[i], 58);
    }
    
    // Base
    fillRect(60, 60, 8, 4, 1);
    
    // Trails and missiles
    for(int i=0; i<MAX_MIS; i++) {
        if (miss[i].active) {
            if (!miss[i].friendly && now % 100 < 50) continue; // dash enemy trails
            drawLine((int)miss[i].sx, (int)miss[i].sy, (int)miss[i].x, (int)miss[i].y, 1);
            if (miss[i].friendly) {
                fillRect((int)miss[i].x-1, (int)miss[i].y-1, 2, 2, 1);
            }
        }
    }
    
    // Explosions
    for(int i=0; i<MAX_EXPL; i++) {
        if (expls[i].active) {
            drawCircle((int)expls[i].x, (int)expls[i].y, (int)expls[i].r, 1);
            if (expls[i].r > 4) {
                // inner clear
                if ((int)expls[i].r % 2 == 0) drawCircle((int)expls[i].x, (int)expls[i].y, (int)expls[i].r-2, 0);
            }
        }
    }
    
    // Cursor
    drawLine(cursorX-3, cursorY, cursorX+3, cursorY, 1);
    drawLine(cursorX, cursorY-3, cursorX, cursorY+3, 1);
    
    // UI
    drawText(2, 2, score);
    drawText(100, 2, ammo);
    
    display_render();
}

}
