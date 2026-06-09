#include "gravity_main.h"
#include "gravity_display.h"
#include "gravity_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Gravity {

struct Spikes {
    float x;
    bool isTop;
    bool active;
};
const int MAX_SPIKES = 6;
Spikes spikes[MAX_SPIKES];

struct Coin {
    float x, y;
    bool active;
};
const int MAX_COINS = 5;
Coin coins[MAX_COINS];

struct Laser {
    float x, y;
    float length;
    float vy;
    bool active;
};
const int MAX_LASERS = 2;
Laser lasers[MAX_LASERS];

struct Particle {
    float x, y, vx, vy;
    float life;
    bool active;
};
const int MAX_PARTICLES = 15;
Particle particles[MAX_PARTICLES];

float py = 32;
float vy = 0;
float gravity = 300.0f; 
int score = 0;
bool gameOver = false;
float speed = 60.0f;

void spawnParticles(float x, float y) {
    for(int i=0; i<5; i++) {
        for(int j=0; j<MAX_PARTICLES; j++) {
            if(!particles[j].active) {
                particles[j] = {x, y, (random(0,200)-100)/2.0f, (gravity>0 ? -1.0f : 1.0f)*(float)random(20,80), 1.0f, true};
                break;
            }
        }
    }
}

void resetGame() {
    py = 32;
    vy = 0;
    gravity = 300.0f;
    score = 0;
    speed = 60.0f;
    gameOver = false;
    for(int i=0; i<MAX_SPIKES; i++) spikes[i].active = false;
    for(int i=0; i<MAX_COINS; i++) coins[i].active = false;
    for(int i=0; i<MAX_LASERS; i++) lasers[i].active = false;
    for(int i=0; i<MAX_PARTICLES; i++) particles[i].active = false;
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
        drawText(40, 30, score);
        display_render();
        if (input_action() && now % 500 < 50) {
            tone(BUZZER_PIN, 800, 100); delay(200);
            resetGame();
        }
        return;
    }
    
    if (input_action()) {
        gravity = -gravity;
        vy = 0;
        spawnParticles(20, py);
        tone(BUZZER_PIN, gravity > 0 ? 400 : 800, 30);
    }
    
    vy += gravity * dt;
    py += vy * dt;
    
    if (py > 58) { py = 58; vy = 0; }
    if (py < 2) { py = 2; vy = 0; }
    
    speed += dt * 2.0f;
    
    for(int i=0; i<MAX_SPIKES; i++) {
        if (spikes[i].active) {
            spikes[i].x -= speed * dt;
            if (spikes[i].x < -10) spikes[i].active = false;
            
            if (spikes[i].x < 24 && spikes[i].x + 10 > 20) {
                if (spikes[i].isTop && py < 17) { gameOver = true; tone(BUZZER_PIN, 100, 300); }
                if (!spikes[i].isTop && py > 43) { gameOver = true; tone(BUZZER_PIN, 100, 300); }
            }
        }
    }
    
    for(int i=0; i<MAX_COINS; i++) {
        if (coins[i].active) {
            coins[i].x -= speed * dt;
            if (coins[i].x < -5) coins[i].active = false;
            if (abs(coins[i].x - 20) < 5 && abs(coins[i].y - py) < 5) {
                coins[i].active = false;
                score += 50;
                tone(BUZZER_PIN, 1500, 30);
            }
        }
    }
    
    for(int i=0; i<MAX_LASERS; i++) {
        if (lasers[i].active) {
            lasers[i].x -= speed * dt;
            lasers[i].y += lasers[i].vy * dt;
            if (lasers[i].y < 15 || lasers[i].y + lasers[i].length > 45) lasers[i].vy = -lasers[i].vy;
            if (lasers[i].x < -5) lasers[i].active = false;
            
            if (lasers[i].x < 24 && lasers[i].x > 16) {
                if (py > lasers[i].y && py < lasers[i].y + lasers[i].length) {
                    gameOver = true;
                    tone(BUZZER_PIN, 100, 300);
                }
            }
        }
    }
    
    for(int i=0; i<MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].x += particles[i].vx * dt;
            particles[i].y += particles[i].vy * dt;
            particles[i].life -= dt * 2.0f;
            if(particles[i].life <= 0) particles[i].active = false;
        }
    }
    
    if (random(0, 40) == 0) {
        bool safe = true;
        for(int j=0; j<MAX_SPIKES; j++) if(spikes[j].active && spikes[j].x > 90) safe = false;
        for(int j=0; j<MAX_LASERS; j++) if(lasers[j].active && lasers[j].x > 90) safe = false;
        
        if (safe) {
            int r = random(0, 10);
            if (r < 5) {
                for(int i=0; i<MAX_SPIKES; i++) {
                    if (!spikes[i].active) {
                        spikes[i] = {128, random(0,2)==0, true};
                        break;
                    }
                }
            } else if (r < 8) {
                for(int i=0; i<MAX_COINS; i++) {
                    if (!coins[i].active) {
                        coins[i] = {128, (float)random(20, 40), true};
                        break;
                    }
                }
            } else {
                for(int i=0; i<MAX_LASERS; i++) {
                    if (!lasers[i].active) {
                        lasers[i] = {128, 20, 20, 30.0f, true};
                        break;
                    }
                }
            }
        }
    }
    
    if(!gameOver && random(0, 10) == 0) score++;
    
    display_clear();
    drawLine(0, 0, 127, 0, 1);
    drawLine(0, 63, 127, 63, 1);
    
    fillRect(20, (int)py, 4, 4, 1);
    
    for(int i=0; i<MAX_PARTICLES; i++) {
        if (particles[i].active) drawPixel((int)particles[i].x, (int)particles[i].y, 1);
    }
    
    for(int i=0; i<MAX_SPIKES; i++) {
        if (spikes[i].active) {
            int x = (int)spikes[i].x;
            if (spikes[i].isTop) {
                drawLine(x, 0, x+5, 15, 1); drawLine(x+10, 0, x+5, 15, 1);
            } else {
                drawLine(x, 63, x+5, 48, 1); drawLine(x+10, 63, x+5, 48, 1);
            }
        }
    }
    
    for(int i=0; i<MAX_COINS; i++) {
        if (coins[i].active) drawCircle((int)coins[i].x, (int)coins[i].y, 3, 1);
    }
    
    for(int i=0; i<MAX_LASERS; i++) {
        if (lasers[i].active) {
            drawLine((int)lasers[i].x, (int)lasers[i].y, (int)lasers[i].x, (int)(lasers[i].y + lasers[i].length), 1);
        }
    }
    
    drawText(2, 2, score / 10);
    display_render();
}

}
