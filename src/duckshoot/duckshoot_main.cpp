#include "duckshoot_main.h"
#include "duckshoot_display.h"
#include "duckshoot_input.h"

#define BUZZER_PIN 14

namespace DuckShoot {

const int SCREEN_W = 128;
const int SCREEN_H = 64;

enum GameState {
    STATE_INTRO,
    STATE_SPAWN_WAVE,
    STATE_PLAYING,
    STATE_WAVE_RESULT,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;
unsigned long stateTimer = 0;

int score = 0;
int roundNum = 1;
int ammo = 3;
int ducksToSpawn = 10;
int hitCount = 0;

float cx = 64.0f;
float cy = 32.0f;

struct Duck {
    float x, y;
    float vx, vy;
    bool active;
    int animState; // 0=fly1, 1=fly2, 2=hit, 3=falling
    unsigned long spawnTime;
};

const int MAX_DUCKS = 2;
Duck ducks[MAX_DUCKS];

void resetGame() {
    score = 0;
    roundNum = 1;
}

void setupWave() {
    ammo = 3;
    int numSpawn = (roundNum >= 3) ? 2 : 1; // 2 ducks starting round 3
    
    for(int i=0; i<MAX_DUCKS; i++) {
        if (i < numSpawn) {
            ducks[i].active = true;
            ducks[i].x = random(20, 108);
            ducks[i].y = 50; // Grass level
            ducks[i].vx = random(0, 2) == 0 ? -20 : 20;
            ducks[i].vy = -20 - (roundNum * 2); // Faster each round
            ducks[i].animState = 0;
            ducks[i].spawnTime = millis();
        } else {
            ducks[i].active = false;
        }
    }
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

unsigned long lastTime = 0;
bool showHitMarker = false;
unsigned long hitMarkerTimer = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    bool fire = input_fire();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(35, 20, "DUCK SHOOT");
        drawText(25, 40, "PRESS TO START");
        display_render();
        
        if (fire) {
            resetGame();
            ducksToSpawn = 10;
            hitCount = 0;
            state = STATE_SPAWN_WAVE;
        }
    } else if (state == STATE_SPAWN_WAVE) {
        display_clear();
        drawText(40, 25, "ROUND");
        drawText(80, 25, roundNum);
        display_render();
        
        if (now - stateTimer > 1500) {
            setupWave();
            state = STATE_PLAYING;
        }
    } else if (state == STATE_PLAYING) {
        
        // Move crosshair
        float ix = input_x();
        float iy = input_y();
        
        cx += ix * 150.0f * dt;
        cy += iy * 150.0f * dt;
        
        if (cx < 0) cx = 0;
        if (cx > SCREEN_W) cx = SCREEN_W;
        if (cy < 0) cy = 0;
        if (cy > SCREEN_H) cy = SCREEN_H;
        
        // Shoot
        if (fire && ammo > 0) {
            ammo--;
            showHitMarker = true;
            hitMarkerTimer = now;
            tone(BUZZER_PIN, 150, 50); // Bang!
            
            // Check hits
            for(int i=0; i<MAX_DUCKS; i++) {
                if (ducks[i].active && ducks[i].animState < 2) {
                    // Duck hitbox is about 12x12
                    if (abs(cx - ducks[i].x) < 8 && abs(cy - ducks[i].y) < 8) {
                        ducks[i].animState = 2; // Hit!
                        ducks[i].vx = 0;
                        ducks[i].vy = 0;
                        ducks[i].spawnTime = now; // reuse for hit timer
                        score += 500;
                        hitCount++;
                        tone(BUZZER_PIN, 1200, 100);
                    }
                }
            }
        }
        
        bool anyAlive = false;
        
        // Update ducks
        for(int i=0; i<MAX_DUCKS; i++) {
            if (ducks[i].active) {
                if (ducks[i].animState < 2) {
                    anyAlive = true;
                    // Flying
                    ducks[i].x += ducks[i].vx * dt;
                    ducks[i].y += ducks[i].vy * dt;
                    
                    // Bounce walls
                    if (ducks[i].x < 5 || ducks[i].x > SCREEN_W-5) ducks[i].vx *= -1;
                    if (ducks[i].y < 5) ducks[i].vy = abs(ducks[i].vy); // bounce down slightly
                    
                    // Random direction change
                    if (random(0, 100) < 5) ducks[i].vx = random(-30, 30);
                    if (random(0, 100) < 5) ducks[i].vy = random(-30, -10); // always bias up
                    
                    // Animate wings
                    ducks[i].animState = (now / 150) % 2;
                    
                    // Fly away timer (5 seconds)
                    if (now - ducks[i].spawnTime > 5000) {
                        ducks[i].active = false; // Flew away
                        tone(BUZZER_PIN, 300, 300);
                    }
                } else if (ducks[i].animState == 2) {
                    anyAlive = true;
                    // Hit pause
                    if (now - ducks[i].spawnTime > 500) {
                        ducks[i].animState = 3; // Falling
                        ducks[i].vy = 50.0f;
                    }
                } else if (ducks[i].animState == 3) {
                    anyAlive = true;
                    // Falling
                    ducks[i].y += ducks[i].vy * dt;
                    if (ducks[i].y > 64) {
                        ducks[i].active = false;
                        tone(BUZZER_PIN, 800, 100);
                    }
                }
            }
        }
        
        if (!anyAlive) {
            // Wave over
            ducksToSpawn -= (roundNum >= 3) ? 2 : 1;
            if (ducksToSpawn <= 0) {
                stateTimer = now;
                state = STATE_WAVE_RESULT;
            } else {
                setupWave(); // Next wave in this round
            }
        } else if (ammo == 0 && ducks[0].animState < 2 && ducks[1].animState < 2) {
            // Out of ammo, ducks fly away quickly
            for(int i=0; i<MAX_DUCKS; i++) {
                if (ducks[i].active && ducks[i].animState < 2) {
                    ducks[i].vy = -80.0f; // Zoom up
                    ducks[i].vx = 0;
                }
            }
        }
        
        // Draw
        display_clear();
        
        // Ducks
        for(int i=0; i<MAX_DUCKS; i++) {
            if (ducks[i].active) {
                int dx = (int)ducks[i].x;
                int dy = (int)ducks[i].y;
                
                if (ducks[i].animState < 2) {
                    // Flying body
                    fillRect(dx-4, dy-2, 8, 4, 1); // body
                    fillRect(dx+(ducks[i].vx>0?4:-6), dy-4, 4, 4, 1); // head
                    // Wings
                    if (ducks[i].animState == 0) drawLine(dx-2, dy-2, dx, dy-6, 1); // Up
                    else drawLine(dx-2, dy+2, dx, dy+6, 1); // Down
                } else if (ducks[i].animState == 2) {
                    // Shocked
                    fillRect(dx-4, dy-4, 8, 8, 1);
                    drawPixel(dx-2, dy-2, 0); drawPixel(dx+2, dy-2, 0);
                } else if (ducks[i].animState == 3) {
                    // Falling
                    fillRect(dx-3, dy-4, 6, 8, 1);
                    drawLine(dx-4, dy-6, dx, dy-2, 1);
                    drawLine(dx+4, dy-6, dx, dy-2, 1);
                }
            }
        }
        
        // Grass foreground
        fillRect(0, 52, SCREEN_W, 12, 1);
        for(int x=2; x<SCREEN_W; x+=8) drawLine(x, 50, x+2, 52, 1);
        
        // HUD
        fillRect(5, 54, 118, 8, 0); // Black box for HUD
        drawText(10, 55, "S:"); drawText(20, 55, score);
        drawText(60, 55, "R:"); drawText(70, 55, roundNum);
        
        // Ammo
        for(int i=0; i<3; i++) {
            if (i < ammo) fillRect(100 + i*6, 56, 3, 5, 1);
            else drawRect(100 + i*6, 56, 3, 5, 1);
        }
        
        // Crosshair
        if (showHitMarker && now - hitMarkerTimer < 100) {
            // Flash crosshair invert
            drawCircle((int)cx, (int)cy, 5, 1);
            fillCircle((int)cx, (int)cy, 3, 1);
        } else {
            drawRect((int)cx-4, (int)cy-4, 9, 9, 1);
            drawLine((int)cx, (int)cy-2, (int)cx, (int)cy+2, 1);
            drawLine((int)cx-2, (int)cy, (int)cx+2, (int)cy, 1);
        }
        
        display_render();
        
    } else if (state == STATE_WAVE_RESULT) {
        display_clear();
        drawText(40, 20, "HITS:");
        drawText(75, 20, hitCount);
        
        if (hitCount >= 6) { // Need 6/10 to pass
            drawText(30, 40, "ROUND CLEAR!");
            display_render();
            if (now - stateTimer > 3000) {
                roundNum++;
                ducksToSpawn = 10;
                hitCount = 0;
                stateTimer = now;
                state = STATE_SPAWN_WAVE;
            }
        } else {
            drawText(35, 40, "GAME OVER");
            display_render();
            if (fire && now - stateTimer > 1000) {
                state = STATE_INTRO;
            }
        }
    } else if (state == STATE_GAMEOVER) {
        // Handled in WAVE_RESULT for simplicity
    }
}

}
