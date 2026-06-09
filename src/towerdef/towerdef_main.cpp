#include "towerdef_main.h"
#include "towerdef_input.h"
#include "towerdef_display.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Towerdef {

enum GameState { STATE_INTRO, STATE_PLAYING, STATE_GAMEOVER };
GameState state = STATE_INTRO;

const int NUM_WP = 6;
float wpx[NUM_WP] = {0, 30, 30, 100, 100, 130};
float wpy[NUM_WP] = {30, 30, 50, 50, 25, 25};

struct Enemy {
    float x, y;
    float hp, maxHp;
    float speed;
    int wpIndex;
    bool active;
};

const int MAX_ENEMIES = 30;
Enemy enemies[MAX_ENEMIES];

struct Spot {
    int x, y;
    int type; // 0=Empty, 1=Archer, 2=Cannon, 3=Mage
    float cooldown;
};
const int NUM_SPOTS = 5;
Spot spots[NUM_SPOTS] = {
    {15, 20, 0, 0},
    {45, 40, 0, 0},
    {80, 60, 0, 0},
    {85, 35, 0, 0},
    {115, 35, 0, 0}
};

struct Proj {
    float x, y, tx, ty;
    int targetEnemy;
    float damage;
    int type;
    bool active;
};
const int MAX_PROJ = 20;
Proj projectiles[MAX_PROJ];

struct Explosion {
    float x, y;
    float radius;
    float life;
    bool active;
};
const int MAX_EXP = 5;
Explosion explosions[MAX_EXP];

int lives = 10;
int gold = 50;
int wave = 1;
int score = 0;

int enemiesToSpawn = 0;
float spawnTimer = 0;
float waveTimer = 0;

int selectedSpot = 0;
bool menuOpen = false;
int menuSelection = 0;

void resetGame() {
    lives = 10;
    gold = 50;
    wave = 1;
    score = 0;
    enemiesToSpawn = 10;
    spawnTimer = 1.0f;
    waveTimer = 0;
    selectedSpot = 0;
    menuOpen = false;
    for(int i=0; i<MAX_ENEMIES; i++) enemies[i].active = false;
    for(int i=0; i<MAX_PROJ; i++) projectiles[i].active = false;
    for(int i=0; i<MAX_EXP; i++) explosions[i].active = false;
    for(int i=0; i<NUM_SPOTS; i++) { spots[i].type = 0; spots[i].cooldown = 0; }
    state = STATE_PLAYING;
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

void spawnEnemy() {
    for(int i=0; i<MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].active = true;
            enemies[i].x = wpx[0];
            enemies[i].y = wpy[0];
            enemies[i].wpIndex = 1;
            enemies[i].maxHp = 10 + wave * 5;
            enemies[i].hp = enemies[i].maxHp;
            enemies[i].speed = 10.0f + wave * 1.5f;
            break;
        }
    }
}

void fireProjectile(int x, int y, int targetIdx, float damage, int type) {
    for(int i=0; i<MAX_PROJ; i++) {
        if(!projectiles[i].active) {
            projectiles[i].active = true;
            projectiles[i].x = x;
            projectiles[i].y = y;
            projectiles[i].targetEnemy = targetIdx;
            projectiles[i].tx = enemies[targetIdx].x;
            projectiles[i].ty = enemies[targetIdx].y;
            projectiles[i].damage = damage;
            projectiles[i].type = type;
            break;
        }
    }
}

void spawnExplosion(float x, float y, float r) {
    for(int i=0; i<MAX_EXP; i++) {
        if(!explosions[i].active) {
            explosions[i].active = true;
            explosions[i].x = x;
            explosions[i].y = y;
            explosions[i].radius = r;
            explosions[i].life = 0.5f;
            break;
        }
    }
}

unsigned long lastTime = 0;
unsigned long stateTimer = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    if (state == STATE_INTRO) {
        display_clear();
        drawRect(0, 0, 128, 64, 1);
        drawRect(2, 2, 124, 60, 1);
        drawText(30, 20, "TOWER DEFENSE");
        
        if ((now / 500) % 2 == 0) {
            drawText(20, 45, "CLICK TO START");
        }
        display_render();
        
        if (input_action()) {
            tone(BUZZER_PIN, 800, 100);
            delay(200);
            resetGame();
        }
        return;
    }
    
    if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(35, 20, "GAME OVER");
        drawText(20, 35, "WAVES SURVIVED:");
        drawText(110, 35, wave);
        if ((now / 500) % 2 == 0) drawText(15, 50, "CLICK TO RESTART");
        display_render();
        if (input_action() && now - stateTimer > 1000) {
            tone(BUZZER_PIN, 800, 100); delay(200);
            resetGame();
        }
        return;
    }
    
    // Controls
    if (!menuOpen) {
        if (input_left()) { selectedSpot--; tone(BUZZER_PIN, 400, 20); delay(150); }
        if (input_right()) { selectedSpot++; tone(BUZZER_PIN, 400, 20); delay(150); }
        if (selectedSpot < 0) selectedSpot = NUM_SPOTS - 1;
        if (selectedSpot >= NUM_SPOTS) selectedSpot = 0;
        
        if (input_action()) {
            if (spots[selectedSpot].type == 0) {
                menuOpen = true;
                menuSelection = 0;
                tone(BUZZER_PIN, 800, 30);
            } else {
                tone(BUZZER_PIN, 300, 30);
            }
        }
    } else {
        if (input_up()) { menuSelection--; tone(BUZZER_PIN, 400, 20); delay(150); }
        if (input_down()) { menuSelection++; tone(BUZZER_PIN, 400, 20); delay(150); }
        if (menuSelection < 0) menuSelection = 2;
        if (menuSelection > 2) menuSelection = 0;
        
        if (input_action()) {
            int cost = (menuSelection == 0) ? 10 : ((menuSelection == 1) ? 20 : 30);
            if (gold >= cost) {
                gold -= cost;
                spots[selectedSpot].type = menuSelection + 1;
                tone(BUZZER_PIN, 1200, 100);
            } else {
                tone(BUZZER_PIN, 200, 100);
            }
            menuOpen = false;
        }
        // Right/Left to cancel
        if (input_right() || input_left()) {
            menuOpen = false;
            tone(BUZZER_PIN, 300, 30);
            delay(150);
        }
    }
    
    // Spawning
    if (enemiesToSpawn > 0) {
        spawnTimer -= dt;
        if (spawnTimer <= 0) {
            spawnEnemy();
            enemiesToSpawn--;
            spawnTimer = 1.0f - (wave * 0.05f);
            if (spawnTimer < 0.2f) spawnTimer = 0.2f;
        }
    } else {
        bool enemiesAlive = false;
        for(int i=0; i<MAX_ENEMIES; i++) if (enemies[i].active) enemiesAlive = true;
        if (!enemiesAlive) {
            waveTimer -= dt;
            if (waveTimer <= 0) {
                wave++;
                enemiesToSpawn = 10 + wave * 2;
                spawnTimer = 1.0f;
                waveTimer = 3.0f; 
                tone(BUZZER_PIN, 1500, 200);
            }
        }
    }
    
    // Enemies Move
    for(int i=0; i<MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            float tx = wpx[enemies[i].wpIndex];
            float ty = wpy[enemies[i].wpIndex];
            float dx = tx - enemies[i].x;
            float dy = ty - enemies[i].y;
            float dist = sqrt(dx*dx + dy*dy);
            
            if (dist < 1.0f) {
                enemies[i].wpIndex++;
                if (enemies[i].wpIndex >= NUM_WP) {
                    enemies[i].active = false;
                    lives--;
                    tone(BUZZER_PIN, 100, 200);
                    if (lives <= 0) {
                        state = STATE_GAMEOVER;
                        stateTimer = now;
                    }
                }
            } else {
                enemies[i].x += (dx/dist) * enemies[i].speed * dt;
                enemies[i].y += (dy/dist) * enemies[i].speed * dt;
            }
        }
    }
    
    // Towers Fire
    for(int i=0; i<NUM_SPOTS; i++) {
        if (spots[i].type > 0) {
            spots[i].cooldown -= dt;
            if (spots[i].cooldown <= 0) {
                int bestTarget = -1;
                float bestDist = 999;
                float range = (spots[i].type == 1) ? 35.0f : ((spots[i].type == 2) ? 45.0f : 55.0f);
                
                for(int e=0; e<MAX_ENEMIES; e++) {
                    if (enemies[e].active) {
                        float dist = sqrt(pow(enemies[e].x - spots[i].x, 2) + pow(enemies[e].y - spots[i].y, 2));
                        if (dist < range && dist < bestDist) {
                            bestDist = dist;
                            bestTarget = e;
                        }
                    }
                }
                
                if (bestTarget != -1) {
                    float dmg = (spots[i].type == 1) ? 5.0f : ((spots[i].type == 2) ? 15.0f : 25.0f);
                    fireProjectile(spots[i].x, spots[i].y, bestTarget, dmg, spots[i].type);
                    spots[i].cooldown = (spots[i].type == 1) ? 0.5f : ((spots[i].type == 2) ? 2.0f : 1.2f);
                    tone(BUZZER_PIN, (spots[i].type == 1) ? 1000 : 400, 20);
                }
            }
        }
    }
    
    // Projectiles
    for(int i=0; i<MAX_PROJ; i++) {
        if (projectiles[i].active) {
            float tx, ty;
            if (projectiles[i].type == 2) {
                tx = projectiles[i].tx;
                ty = projectiles[i].ty;
            } else {
                int e = projectiles[i].targetEnemy;
                if (enemies[e].active) {
                    tx = enemies[e].x;
                    ty = enemies[e].y;
                } else {
                    tx = projectiles[i].tx;
                    ty = projectiles[i].ty;
                }
            }
            
            float dx = tx - projectiles[i].x;
            float dy = ty - projectiles[i].y;
            float dist = sqrt(dx*dx + dy*dy);
            float speed = (projectiles[i].type == 1) ? 80.0f : 50.0f;
            
            if (dist < 2.0f) {
                projectiles[i].active = false;
                if (projectiles[i].type == 2) {
                    spawnExplosion(tx, ty, 20.0f);
                    tone(BUZZER_PIN, 200, 50);
                    for(int e=0; e<MAX_ENEMIES; e++) {
                        if (enemies[e].active) {
                            float ed = sqrt(pow(enemies[e].x - tx, 2) + pow(enemies[e].y - ty, 2));
                            if (ed < 20.0f) {
                                enemies[e].hp -= projectiles[i].damage;
                                if (enemies[e].hp <= 0) {
                                    enemies[e].active = false;
                                    gold += 2; score += 10;
                                }
                            }
                        }
                    }
                } else {
                    int e = projectiles[i].targetEnemy;
                    if (enemies[e].active) {
                        enemies[e].hp -= projectiles[i].damage;
                        if (enemies[e].hp <= 0) {
                            enemies[e].active = false;
                            gold += 2; score += 10;
                            tone(BUZZER_PIN, 1200, 30);
                        }
                    }
                }
            } else {
                projectiles[i].x += (dx/dist) * speed * dt;
                projectiles[i].y += (dy/dist) * speed * dt;
            }
        }
    }
    
    for(int i=0; i<MAX_EXP; i++) {
        if (explosions[i].active) {
            explosions[i].life -= dt;
            if (explosions[i].life <= 0) explosions[i].active = false;
        }
    }
    
    display_clear();
    
    // Path
    for(int i=0; i<NUM_WP-1; i++) {
        drawLine((int)wpx[i], (int)wpy[i], (int)wpx[i+1], (int)wpy[i+1], 1);
        drawLine((int)wpx[i], (int)wpy[i]-1, (int)wpx[i+1], (int)wpy[i+1]-1, 1);
        drawLine((int)wpx[i], (int)wpy[i]+1, (int)wpx[i+1], (int)wpy[i+1]+1, 1);
    }
    
    // Spots & Towers
    for(int i=0; i<NUM_SPOTS; i++) {
        if (spots[i].type == 0) {
            drawCircle(spots[i].x, spots[i].y, 4, 1);
        } else if (spots[i].type == 1) { // Archer
            drawLine(spots[i].x-3, spots[i].y+3, spots[i].x+3, spots[i].y+3, 1);
            drawLine(spots[i].x-3, spots[i].y+3, spots[i].x, spots[i].y-4, 1);
            drawLine(spots[i].x+3, spots[i].y+3, spots[i].x, spots[i].y-4, 1);
        } else if (spots[i].type == 2) { // Cannon
            drawCircle(spots[i].x, spots[i].y, 4, 1);
            fillRect(spots[i].x-2, spots[i].y-2, 5, 5, 1);
        } else if (spots[i].type == 3) { // Mage
            drawLine(spots[i].x-4, spots[i].y, spots[i].x+4, spots[i].y, 1);
            drawLine(spots[i].x, spots[i].y-4, spots[i].x, spots[i].y+4, 1);
            drawCircle(spots[i].x, spots[i].y, 2, 1);
        }
        
        if (selectedSpot == i && !menuOpen) {
            drawRect(spots[i].x-6, spots[i].y-6, 13, 13, 1);
        }
    }
    
    // Enemies
    for(int i=0; i<MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            fillRect((int)enemies[i].x - 2, (int)enemies[i].y - 2, 5, 5, 1);
            if (enemies[i].hp < enemies[i].maxHp) {
                int hpW = (int)((enemies[i].hp / enemies[i].maxHp) * 6.0f);
                drawLine((int)enemies[i].x - 3, (int)enemies[i].y - 5, (int)enemies[i].x - 3 + hpW, (int)enemies[i].y - 5, 1);
            }
        }
    }
    
    // Proj & Exp
    for(int i=0; i<MAX_PROJ; i++) {
        if (projectiles[i].active) {
            if (projectiles[i].type == 2) drawCircle((int)projectiles[i].x, (int)projectiles[i].y, 2, 1);
            else drawRect((int)projectiles[i].x, (int)projectiles[i].y, 2, 2, 1);
        }
    }
    for(int i=0; i<MAX_EXP; i++) {
        if (explosions[i].active) {
            int r = (int)(explosions[i].radius * (0.5f - explosions[i].life)*2);
            drawCircle((int)explosions[i].x, (int)explosions[i].y, r, 1);
        }
    }
    
    // UI Bar
    drawLine(0, 9, 128, 9, 1);
    drawText(2, 1, "G:"); drawText(15, 1, gold);
    drawText(45, 1, "L:"); drawText(58, 1, lives);
    drawText(85, 1, "W:"); drawText(98, 1, wave);
    
    // Build Menu
    if (menuOpen) {
        int mx = spots[selectedSpot].x - 25;
        int my = spots[selectedSpot].y + 10;
        if (mx < 0) mx = 0; if (mx > 65) mx = 65;
        if (my > 30) my = spots[selectedSpot].y - 35;
        
        fillRect(mx, my, 55, 32, 0);
        drawRect(mx, my, 55, 32, 1);
        
        drawText(mx+2, my+2, "Arc 10");
        drawText(mx+2, my+12, "Can 20");
        drawText(mx+2, my+22, "Mag 30");
        
        drawText(mx+45, my+2 + (menuSelection*10), "<");
    }
    
    display_render();
}

}
