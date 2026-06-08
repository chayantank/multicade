#include "balloon_main.h"
#include "balloon_display.h"
#include "balloon_input.h"

#define BUZZER_PIN 14

namespace Balloon {

const int SCREEN_W = 128;
const int SCREEN_H = 64;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;
unsigned long stateTimer = 0;

struct Entity {
    float x, y;
    float vx, vy;
    bool active;
    int state; // 0=normal, 1=parachute (for enemies)
};

Entity player;
int score = 0;
int wave = 1;

const int MAX_ENEMIES = 5;
Entity enemies[MAX_ENEMIES];

void spawnWave() {
    for (int i=0; i<MAX_ENEMIES; i++) enemies[i].active = false;
    
    int count = min(wave, MAX_ENEMIES);
    for (int i=0; i<count; i++) {
        enemies[i].active = true;
        enemies[i].x = random(10, 118);
        enemies[i].y = random(10, 50);
        enemies[i].vx = random(0, 2) == 0 ? -15 : 15;
        enemies[i].vy = 0;
        enemies[i].state = 0;
    }
}

void resetGame() {
    score = 0;
    wave = 1;
    player.x = 64;
    player.y = 32;
    player.vx = 0;
    player.vy = 0;
    player.active = true;
    spawnWave();
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
    
    bool flap = input_flap();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(15, 20, "BALLOON JOUST");
        drawText(20, 40, "PRESS TO START");
        display_render();
        
        if (flap) {
            resetGame();
            state = STATE_PLAYING;
        }
    } else if (state == STATE_PLAYING) {
        
        // --- Player Physics ---
        float ix = input_x();
        player.vx += ix * 150.0f * dt;
        player.vy += 60.0f * dt; // Gravity
        
        if (flap) {
            player.vy = -40.0f;
            tone(BUZZER_PIN, 1500, 20);
        }
        
        // Friction / Drag
        player.vx *= 0.98f;
        if (player.vy > 60.0f) player.vy = 60.0f; // Terminal velocity
        
        player.x += player.vx * dt;
        player.y += player.vy * dt;
        
        // Screen Wrap X
        if (player.x < 0) player.x += SCREEN_W;
        if (player.x >= SCREEN_W) player.x -= SCREEN_W;
        
        // Boundaries Y
        if (player.y < 5) {
            player.y = 5;
            player.vy = 0;
        }
        if (player.y > SCREEN_H - 5) {
            // Water = death
            state = STATE_GAMEOVER;
            stateTimer = now;
            tone(BUZZER_PIN, 100, 1000);
        }
        
        // --- Enemy Logic ---
        bool allDead = true;
        for (int i=0; i<MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                allDead = false;
                
                if (enemies[i].state == 0) { // Normal flying
                    enemies[i].vy += 60.0f * dt; // Gravity
                    
                    // Simple flap AI: try to stay in middle Y
                    if (enemies[i].y > 40 && random(0, 100) < 10) {
                        enemies[i].vy = -35.0f;
                    }
                    
                    enemies[i].x += enemies[i].vx * dt;
                    enemies[i].y += enemies[i].vy * dt;
                    
                    // Screen Wrap
                    if (enemies[i].x < 0) enemies[i].x += SCREEN_W;
                    if (enemies[i].x >= SCREEN_W) enemies[i].x -= SCREEN_W;
                    
                    if (enemies[i].y < 5) { enemies[i].y = 5; enemies[i].vy = 0; }
                    if (enemies[i].y > SCREEN_H - 10) enemies[i].vy = -40.0f;
                    
                    // Collision with player
                    if (abs(player.x - enemies[i].x) < 8 && abs(player.y - enemies[i].y) < 8) {
                        if (player.y < enemies[i].y - 2) {
                            // Player hit enemy from above
                            enemies[i].state = 1; // Parachute
                            enemies[i].vx = 0;
                            enemies[i].vy = 20.0f;
                            score += 100;
                            player.vy = -30.0f; // Bounce
                            tone(BUZZER_PIN, 2000, 50);
                        } else if (enemies[i].y < player.y - 2) {
                            // Enemy hit player from above
                            state = STATE_GAMEOVER;
                            stateTimer = now;
                            tone(BUZZER_PIN, 100, 1000);
                        } else {
                            // Clash (bounce off)
                            float temp = player.vx;
                            player.vx = enemies[i].vx;
                            enemies[i].vx = temp;
                            tone(BUZZER_PIN, 400, 20);
                        }
                    }
                } else if (enemies[i].state == 1) { // Parachute falling
                    enemies[i].y += enemies[i].vy * dt;
                    if (enemies[i].y > SCREEN_H) {
                        enemies[i].active = false;
                    }
                    // Player can hit them again for points
                    if (abs(player.x - enemies[i].x) < 8 && abs(player.y - enemies[i].y) < 8) {
                        enemies[i].active = false;
                        score += 300;
                        tone(BUZZER_PIN, 2500, 50);
                    }
                }
            }
        }
        
        if (allDead) {
            wave++;
            spawnWave();
            tone(BUZZER_PIN, 1500, 200);
        }
        
        // --- Render ---
        display_clear();
        
        // Draw Water
        for(int x=0; x<SCREEN_W; x+=4) {
            drawPixel(x, SCREEN_H-2 + (int)(millis()/200 % 2), 1);
        }
        
        // Draw Player
        int px = (int)player.x;
        int py = (int)player.y;
        fillRect(px-3, py, 6, 4, 1); // Body
        drawCircle(px, py-6, 3, 1); // Balloon
        drawLine(px-1, py-3, px-2, py, 1); // String
        drawLine(px+1, py-3, px+2, py, 1);
        
        // Draw Enemies
        for (int i=0; i<MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                int ex = (int)enemies[i].x;
                int ey = (int)enemies[i].y;
                if (enemies[i].state == 0) {
                    drawRect(ex-3, ey, 6, 4, 1);
                    drawCircle(ex, ey-6, 3, 1); // Balloon
                    drawLine(ex, ey-3, ex, ey, 1);
                } else {
                    drawRect(ex-3, ey, 6, 4, 1);
                    // Parachute
                    drawLine(ex-4, ey-4, ex+4, ey-4, 1);
                    drawLine(ex-4, ey-4, ex-2, ey, 1);
                    drawLine(ex+4, ey-4, ex+2, ey, 1);
                }
            }
        }
        
        // Draw HUD
        drawText(2, 2, score);
        
        display_render();
        
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(35, 20, "GAME OVER");
        drawText(35, 40, "SCORE:");
        drawText(75, 40, score);
        display_render();
        if (flap && now - stateTimer > 1000) state = STATE_INTRO;
    }
}

}
