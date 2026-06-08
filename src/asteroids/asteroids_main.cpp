#include "asteroids_main.h"
#include "asteroids_display.h"
#include "asteroids_input.h"

#define BUZZER_PIN 14

namespace Asteroids {

const int SCREEN_W = 128;
const int SCREEN_H = 64;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_DEATH,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;

struct Ship {
    float x, y;
    float vx, vy;
    float angle; // in radians
} ship;

const int MAX_BULLETS = 5;
struct Bullet {
    float x, y;
    float vx, vy;
    float life; // time remaining
    bool active;
};
Bullet bullets[MAX_BULLETS];

const int MAX_ASTEROIDS = 24;
struct Asteroid {
    float x, y;
    float vx, vy;
    int size; // 3=Large, 2=Medium, 1=Small
    bool active;
};
Asteroid asteroids[MAX_ASTEROIDS];

int score = 0;
int lives = 3;
unsigned long stateTimer = 0;
unsigned long lastFrameTime = 0;
bool lastFire = false;

void spawnAsteroid(float x, float y, int size) {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!asteroids[i].active) {
            asteroids[i].x = x;
            asteroids[i].y = y;
            asteroids[i].size = size;
            
            float angle = random(0, 360) * PI / 180.0f;
            float speed = 10.0f + (4 - size) * 10.0f; // Smaller = faster
            asteroids[i].vx = cos(angle) * speed;
            asteroids[i].vy = sin(angle) * speed;
            asteroids[i].active = true;
            break;
        }
    }
}

void resetLevel(bool fullReset) {
    if (fullReset) {
        score = 0;
        lives = 3;
        for (int i = 0; i < MAX_ASTEROIDS; i++) asteroids[i].active = false;
        spawnAsteroid(0, 0, 3);
        spawnAsteroid(SCREEN_W, 0, 3);
        spawnAsteroid(0, SCREEN_H, 3);
    }
    
    ship.x = SCREEN_W / 2;
    ship.y = SCREEN_H / 2;
    ship.vx = 0;
    ship.vy = 0;
    ship.angle = -PI / 2.0f; // Pointing up
    
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

void wrapCoordinates(float &x, float &y) {
    if (x < 0) x += SCREEN_W;
    else if (x >= SCREEN_W) x -= SCREEN_W;
    
    if (y < 0) y += SCREEN_H;
    else if (y >= SCREEN_H) y -= SCREEN_H;
}

void drawShip() {
    // Ship is a triangle
    float headX = ship.x + cos(ship.angle) * 6.0f;
    float headY = ship.y + sin(ship.angle) * 6.0f;
    
    float leftX = ship.x + cos(ship.angle + 2.5f) * 5.0f;
    float leftY = ship.y + sin(ship.angle + 2.5f) * 5.0f;
    
    float rightX = ship.x + cos(ship.angle - 2.5f) * 5.0f;
    float rightY = ship.y + sin(ship.angle - 2.5f) * 5.0f;
    
    drawLine(headX, headY, leftX, leftY, 1);
    drawLine(leftX, leftY, rightX, rightY, 1);
    drawLine(rightX, rightY, headX, headY, 1);
    
    // Draw thrust flame if accelerating
    if (input_up()) {
        float flameX = ship.x + cos(ship.angle + PI) * 6.0f;
        float flameY = ship.y + sin(ship.angle + PI) * 6.0f;
        drawLine((ship.x + leftX)/2, (ship.y + leftY)/2, flameX, flameY, 1);
        drawLine((ship.x + rightX)/2, (ship.y + rightY)/2, flameX, flameY, 1);
    }
}

void loopPlaying(float dt) {
    bool currentFire = input_fire();
    
    // 1. Ship Input
    if (input_left()) ship.angle -= 4.0f * dt;
    if (input_right()) ship.angle += 4.0f * dt;
    
    if (input_up()) {
        ship.vx += cos(ship.angle) * 80.0f * dt;
        ship.vy += sin(ship.angle) * 80.0f * dt;
        // Small engine hum could go here, but omitted to save ears
    }
    
    // Friction
    ship.vx *= 0.99f;
    ship.vy *= 0.99f;
    
    ship.x += ship.vx * dt;
    ship.y += ship.vy * dt;
    wrapCoordinates(ship.x, ship.y);
    
    // 2. Fire Bullets
    if (currentFire && !lastFire) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) {
                bullets[i].active = true;
                bullets[i].x = ship.x + cos(ship.angle) * 6.0f;
                bullets[i].y = ship.y + sin(ship.angle) * 6.0f;
                bullets[i].vx = ship.vx + cos(ship.angle) * 120.0f;
                bullets[i].vy = ship.vy + sin(ship.angle) * 120.0f;
                bullets[i].life = 1.0f; // 1 second life
                tone(BUZZER_PIN, 1200, 20);
                break;
            }
        }
    }
    lastFire = currentFire;
    
    // 3. Update Bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].x += bullets[i].vx * dt;
            bullets[i].y += bullets[i].vy * dt;
            wrapCoordinates(bullets[i].x, bullets[i].y);
            
            bullets[i].life -= dt;
            if (bullets[i].life <= 0) bullets[i].active = false;
        }
    }
    
    // 4. Update Asteroids & Collisions
    int activeCount = 0;
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].active) {
            activeCount++;
            asteroids[i].x += asteroids[i].vx * dt;
            asteroids[i].y += asteroids[i].vy * dt;
            wrapCoordinates(asteroids[i].x, asteroids[i].y);
            
            float radius = asteroids[i].size * 4.0f;
            
            // Collision with ship
            float dx = ship.x - asteroids[i].x;
            float dy = ship.y - asteroids[i].y;
            if (dx*dx + dy*dy < (radius + 4.0f)*(radius + 4.0f)) {
                state = STATE_DEATH;
                stateTimer = millis();
                tone(BUZZER_PIN, 100, 800);
                return;
            }
            
            // Collision with bullets
            for (int b = 0; b < MAX_BULLETS; b++) {
                if (bullets[b].active) {
                    float bdx = bullets[b].x - asteroids[i].x;
                    float bdy = bullets[b].y - asteroids[i].y;
                    if (bdx*bdx + bdy*bdy < radius*radius) {
                        // Hit!
                        bullets[b].active = false;
                        asteroids[i].active = false;
                        score += (4 - asteroids[i].size) * 10;
                        tone(BUZZER_PIN, 300 + (3-asteroids[i].size)*200, 50);
                        
                        if (asteroids[i].size > 1) {
                            spawnAsteroid(asteroids[i].x, asteroids[i].y, asteroids[i].size - 1);
                            spawnAsteroid(asteroids[i].x, asteroids[i].y, asteroids[i].size - 1);
                        }
                        break; // Move to next asteroid
                    }
                }
            }
        }
    }
    
    // Respawn level if cleared
    if (activeCount == 0) {
        score += 100;
        tone(BUZZER_PIN, 1500, 300);
        spawnAsteroid(0, 0, 3);
        spawnAsteroid(SCREEN_W, 0, 3);
        spawnAsteroid(0, SCREEN_H, 3);
        spawnAsteroid(SCREEN_W, SCREEN_H, 3); // Slightly harder
    }
    
    // Draw
    display_clear();
    
    drawShip();
    
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) fillRect((int)bullets[i].x, (int)bullets[i].y, 2, 2, 1);
    }
    
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].active) {
            // Draw asteroid as a simple diamond/box based on size
            int r = asteroids[i].size * 4;
            drawLine(asteroids[i].x, asteroids[i].y - r, asteroids[i].x + r, asteroids[i].y, 1);
            drawLine(asteroids[i].x + r, asteroids[i].y, asteroids[i].x, asteroids[i].y + r, 1);
            drawLine(asteroids[i].x, asteroids[i].y + r, asteroids[i].x - r, asteroids[i].y, 1);
            drawLine(asteroids[i].x - r, asteroids[i].y, asteroids[i].x, asteroids[i].y - r, 1);
        }
    }
    
    // Draw HUD
    drawText(2, 1, "S:"); drawText(14, 1, score);
    drawText(100, 1, "L:"); drawText(112, 1, lives);
    
    display_render();
}

void loop() {
    unsigned long now = millis();
    float dt = (now - lastFrameTime) / 1000.0f;
    lastFrameTime = now;
    if (dt > 0.1f) dt = 0.1f;

    bool currentFire = input_fire();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(35, 20, "ASTEROIDS");
        drawText(30, 40, "PRESS FIRE");
        display_render();

        if (currentFire && !lastFire) {
            resetLevel(true);
            state = STATE_PLAYING;
            lastFrameTime = millis();
        }
    } else if (state == STATE_PLAYING) {
        loopPlaying(dt);
    } else if (state == STATE_DEATH) {
        display_clear();
        drawText(35, 20, "SHIP DESTROYED");
        display_render();

        if (millis() - stateTimer > 1500) {
            lives--;
            if (lives <= 0) {
                state = STATE_GAMEOVER;
                tone(BUZZER_PIN, 100, 600);
            } else {
                resetLevel(false);
                state = STATE_PLAYING;
                lastFrameTime = millis();
            }
        }
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(35, 20, "GAME OVER");
        drawText(40, 40, "SCORE:");
        drawText(80, 40, score);
        display_render();

        if (currentFire && !lastFire) {
            state = STATE_INTRO;
        }
    }
    lastFire = currentFire;
}

}
