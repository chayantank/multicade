#include "starfighter_main.h"
#include "starfighter_display.h"
#include "starfighter_input.h"

#define BUZZER_PIN 14

namespace Starfighter {

const int SCREEN_W = 64;
const int SCREEN_H = 128;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;

// --- Stars ---
const int NUM_STARS = 20;
struct Star {
    float x, y, speed;
} stars[NUM_STARS];

// --- Player ---
float px = 32.0f;
float py = 110.0f;
int weaponLevel = 1;
int score = 0;
int lives = 3;
unsigned long lastShootTime = 0;
unsigned long stateTimer = 0;

// --- Bullets ---
const int MAX_BULLETS = 10;
struct Bullet {
    float x, y, vx, vy;
    bool active;
} bullets[MAX_BULLETS];

// --- Enemies ---
const int MAX_ENEMIES = 5;
struct Enemy {
    float x, y;
    float baseX;
    float speed;
    float phase;
    int hp;
    bool active;
} enemies[MAX_ENEMIES];

// --- Particles ---
const int MAX_PARTICLES = 20;
struct Particle {
    float x, y, vx, vy, life;
    bool active;
} particles[MAX_PARTICLES];

// --- Upgrades ---
struct Powerup {
    float x, y;
    bool active;
} powerup;

void spawnParticles(float x, float y, int count) {
    for (int i = 0; i < MAX_PARTICLES && count > 0; i++) {
        if (!particles[i].active) {
            particles[i].active = true;
            particles[i].x = x;
            particles[i].y = y;
            float angle = random(0, 360) * 3.14159f / 180.0f;
            float speed = random(10, 40);
            particles[i].vx = cos(angle) * speed;
            particles[i].vy = sin(angle) * speed;
            particles[i].life = random(10, 30) / 10.0f;
            count--;
        }
    }
}

void resetGame() {
    score = 0;
    lives = 3;
    weaponLevel = 1;
    px = 32.0f;
    py = 110.0f;
    for(int i=0; i<MAX_BULLETS; i++) bullets[i].active = false;
    for(int i=0; i<MAX_ENEMIES; i++) enemies[i].active = false;
    for(int i=0; i<MAX_PARTICLES; i++) particles[i].active = false;
    powerup.active = false;
    
    for(int i=0; i<NUM_STARS; i++) {
        stars[i].x = random(0, SCREEN_W);
        stars[i].y = random(0, SCREEN_H);
        stars[i].speed = random(10, 30);
    }
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
    
    bool fire = input_fire();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(0, 40, "STARFIGHTER");
        drawText(5, 70, "PRESS TO");
        drawText(15, 80, "START");
        display_render();
        
        if (fire) {
            resetGame();
            state = STATE_PLAYING;
        }
    } else if (state == STATE_PLAYING) {
        // --- Input ---
        px += input_x() * 60.0f * dt;
        py += input_y() * 60.0f * dt;
        
        if (px < 4) px = 4;
        if (px > SCREEN_W - 4) px = SCREEN_W - 4;
        if (py < 4) py = 4;
        if (py > SCREEN_H - 4) py = SCREEN_H - 4;
        
        if (fire && now - lastShootTime > 200) {
            lastShootTime = now;
            tone(BUZZER_PIN, 1200, 20);
            
            if (weaponLevel == 1) {
                for(int i=0; i<MAX_BULLETS; i++) {
                    if(!bullets[i].active) {
                        bullets[i].active = true;
                        bullets[i].x = px; bullets[i].y = py - 4;
                        bullets[i].vx = 0; bullets[i].vy = -100;
                        break;
                    }
                }
            } else if (weaponLevel == 2) {
                int spawned = 0;
                for(int i=0; i<MAX_BULLETS && spawned < 2; i++) {
                    if(!bullets[i].active) {
                        bullets[i].active = true;
                        bullets[i].x = px + (spawned == 0 ? -3 : 3);
                        bullets[i].y = py - 4;
                        bullets[i].vx = 0; bullets[i].vy = -100;
                        spawned++;
                    }
                }
            } else {
                int spawned = 0;
                for(int i=0; i<MAX_BULLETS && spawned < 3; i++) {
                    if(!bullets[i].active) {
                        bullets[i].active = true;
                        bullets[i].x = px; bullets[i].y = py - 4;
                        if(spawned == 0) { bullets[i].vx = 0; bullets[i].vy = -100; }
                        else if(spawned == 1) { bullets[i].vx = -30; bullets[i].vy = -95; }
                        else { bullets[i].vx = 30; bullets[i].vy = -95; }
                        spawned++;
                    }
                }
            }
        }
        
        // --- Spawn Enemies ---
        if (random(0, 100) < 2 + (score/10)) {
            for(int i=0; i<MAX_ENEMIES; i++) {
                if(!enemies[i].active) {
                    enemies[i].active = true;
                    enemies[i].baseX = random(10, SCREEN_W - 10);
                    enemies[i].x = enemies[i].baseX;
                    enemies[i].y = -10;
                    enemies[i].speed = random(15, 35) + (score * 0.5f);
                    enemies[i].phase = random(0, 100) / 10.0f;
                    enemies[i].hp = 1 + (score / 20);
                    break;
                }
            }
        }
        
        // --- Update Stars ---
        for(int i=0; i<NUM_STARS; i++) {
            stars[i].y += stars[i].speed * dt;
            if(stars[i].y > SCREEN_H) {
                stars[i].y = 0;
                stars[i].x = random(0, SCREEN_W);
            }
        }
        
        // --- Update Bullets ---
        for(int i=0; i<MAX_BULLETS; i++) {
            if(bullets[i].active) {
                bullets[i].x += bullets[i].vx * dt;
                bullets[i].y += bullets[i].vy * dt;
                if(bullets[i].y < 0 || bullets[i].x < 0 || bullets[i].x > SCREEN_W) bullets[i].active = false;
            }
        }
        
        // --- Update Particles ---
        for(int i=0; i<MAX_PARTICLES; i++) {
            if(particles[i].active) {
                particles[i].x += particles[i].vx * dt;
                particles[i].y += particles[i].vy * dt;
                particles[i].life -= dt;
                if(particles[i].life <= 0) particles[i].active = false;
            }
        }
        
        // --- Update Powerup ---
        if (powerup.active) {
            powerup.y += 20 * dt;
            if (powerup.y > SCREEN_H) powerup.active = false;
            
            // Collision with player
            if (abs(powerup.x - px) < 8 && abs(powerup.y - py) < 8) {
                powerup.active = false;
                if (weaponLevel < 3) weaponLevel++;
                else score += 5;
                tone(BUZZER_PIN, 2000, 100);
            }
        }
        
        // --- Update Enemies & Collisions ---
        for(int i=0; i<MAX_ENEMIES; i++) {
            if(enemies[i].active) {
                enemies[i].y += enemies[i].speed * dt;
                enemies[i].phase += dt * 3.0f;
                enemies[i].x = enemies[i].baseX + sin(enemies[i].phase) * 15.0f;
                
                if(enemies[i].y > SCREEN_H) enemies[i].active = false;
                
                // Player collision
                if (abs(enemies[i].x - px) < 6 && abs(enemies[i].y - py) < 6) {
                    enemies[i].active = false;
                    spawnParticles(px, py, 10);
                    lives--;
                    weaponLevel = 1;
                    tone(BUZZER_PIN, 100, 500);
                    if (lives <= 0) {
                        state = STATE_GAMEOVER;
                        stateTimer = now;
                    }
                }
                
                // Bullet collision
                for(int b=0; b<MAX_BULLETS; b++) {
                    if (bullets[b].active && enemies[i].active) {
                        if (abs(bullets[b].x - enemies[i].x) < 5 && abs(bullets[b].y - enemies[i].y) < 5) {
                            bullets[b].active = false;
                            enemies[i].hp--;
                            if (enemies[i].hp <= 0) {
                                enemies[i].active = false;
                                score++;
                                spawnParticles(enemies[i].x, enemies[i].y, 5);
                                tone(BUZZER_PIN, 800, 30);
                                
                                // Drop powerup?
                                if (!powerup.active && random(0, 10) == 0) {
                                    powerup.active = true;
                                    powerup.x = enemies[i].x;
                                    powerup.y = enemies[i].y;
                                }
                            } else {
                                tone(BUZZER_PIN, 400, 10);
                            }
                        }
                    }
                }
            }
        }
        
        // --- Draw ---
        display_clear();
        
        // Stars
        for(int i=0; i<NUM_STARS; i++) {
            drawPixel((int)stars[i].x, (int)stars[i].y, 1);
        }
        
        // Particles
        for(int i=0; i<MAX_PARTICLES; i++) {
            if(particles[i].active) drawPixel((int)particles[i].x, (int)particles[i].y, 1);
        }
        
        // Powerup
        if (powerup.active) {
            drawCircle((int)powerup.x, (int)powerup.y, 3, 1);
            drawPixel((int)powerup.x, (int)powerup.y, 1);
        }
        
        // Bullets
        for(int i=0; i<MAX_BULLETS; i++) {
            if(bullets[i].active) {
                drawLine((int)bullets[i].x, (int)bullets[i].y, (int)bullets[i].x, (int)bullets[i].y+2, 1);
            }
        }
        
        // Enemies (TIE-fighter shape)
        for(int i=0; i<MAX_ENEMIES; i++) {
            if(enemies[i].active) {
                int ex = (int)enemies[i].x;
                int ey = (int)enemies[i].y;
                fillCircle(ex, ey, 2, 1);
                drawLine(ex-3, ey-2, ex-3, ey+2, 1);
                drawLine(ex+3, ey-2, ex+3, ey+2, 1);
            }
        }
        
        // Player (X-wing shape)
        int cx = (int)px;
        int cy = (int)py;
        drawLine(cx, cy-4, cx, cy+4, 1); // fuselage
        drawLine(cx-4, cy+2, cx+4, cy+2, 1); // wings
        drawLine(cx-4, cy, cx-4, cy+4, 1); // left gun
        drawLine(cx+4, cy, cx+4, cy+4, 1); // right gun
        
        // HUD
        drawText(0, 0, score);
        for(int i=0; i<lives; i++) {
            fillRect(SCREEN_W - 4 - (i*5), 2, 3, 3, 1);
        }
        
        display_render();
        
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(5, 30, "GAME OVER");
        drawText(15, 50, "SCORE:");
        drawText(30, 65, score);
        display_render();
        if (fire && now - stateTimer > 1000) {
            state = STATE_INTRO;
        }
    }
}

}
