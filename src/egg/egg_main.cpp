#include "egg_main.h"
#include "egg_display.h"
#include "egg_input.h"

#define BUZZER_PIN 14

namespace Egg {

const int SCREEN_W = 64;
const int SCREEN_H = 128;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;

float basketX = 24.0f;
const float basketW = 16.0f;
const float basketH = 8.0f;
const float basketSpeed = 60.0f; 

enum DropType { NORMAL, GOLDEN, BOMB };

struct Drop {
    float x;
    float y;
    float vy;
    DropType type;
    bool active;
};

const int MAX_DROPS = 4;
Drop drops[MAX_DROPS];

int score = 0;
int lives = 3;
float wind = 0;
int windTimer = 0;

unsigned long lastTime = 0;
unsigned long spawnTimer = 0;
unsigned long stateTimer = 0;

void resetGame() {
    score = 0;
    lives = 3;
    basketX = 24.0f;
    wind = 0;
    for(int i=0; i<MAX_DROPS; i++) {
        drops[i].active = false;
    }
    spawnTimer = millis();
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    bool fire = input_fire();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(5, 40, "EGG CATCH");
        drawText(5, 70, "PRESS TO");
        drawText(15, 80, "START");
        display_render();
        
        if (fire) {
            resetGame();
            state = STATE_PLAYING;
        }
    } else if (state == STATE_PLAYING) {
        
        windTimer--;
        if (windTimer <= 0) {
            windTimer = random(50, 200);
            wind = (random(0, 100) - 50) / 2.0f; 
        }
        
        if (input_left()) {
            basketX -= basketSpeed * dt;
        } else if (input_right()) {
            basketX += basketSpeed * dt;
        }
        
        if (basketX < 0) basketX = 0;
        if (basketX > SCREEN_W - basketW) basketX = SCREEN_W - basketW;

        int spawnDelay = 2000 - (score * 50);
        if (spawnDelay < 400) spawnDelay = 400;
        
        if (now - spawnTimer > spawnDelay) {
            for(int i=0; i<MAX_DROPS; i++) {
                if (!drops[i].active) {
                    drops[i].active = true;
                    drops[i].x = random(4, SCREEN_W - 4);
                    drops[i].y = -5;
                    drops[i].vy = 20.0f + (score * 1.5f);
                    
                    int r = random(0, 100);
                    if (r < 10) drops[i].type = GOLDEN;
                    else if (r < 30) drops[i].type = BOMB;
                    else drops[i].type = NORMAL;
                    
                    break;
                }
            }
            spawnTimer = now;
        }

        for(int i=0; i<MAX_DROPS; i++) {
            if (drops[i].active) {
                drops[i].y += drops[i].vy * dt;
                drops[i].x += wind * dt;
                
                if (drops[i].x < 2) drops[i].x = 2;
                if (drops[i].x > SCREEN_W - 2) drops[i].x = SCREEN_W - 2;
                
                if (drops[i].y > 110 && drops[i].y < 120) {
                    if (drops[i].x >= basketX && drops[i].x <= basketX + basketW) {
                        drops[i].active = false;
                        if (drops[i].type == NORMAL) {
                            score++;
                            tone(BUZZER_PIN, 1000, 50);
                        } else if (drops[i].type == GOLDEN) {
                            score += 5;
                            if (lives < 5) lives++;
                            tone(BUZZER_PIN, 2000, 100);
                        } else if (drops[i].type == BOMB) {
                            lives--;
                            tone(BUZZER_PIN, 100, 400);
                            if (lives <= 0) {
                                state = STATE_GAMEOVER;
                                stateTimer = now;
                                tone(BUZZER_PIN, 150, 800);
                            }
                        }
                    }
                }
                
                if (drops[i].y > SCREEN_H) {
                    drops[i].active = false;
                    if (drops[i].type == NORMAL || drops[i].type == GOLDEN) {
                        lives--;
                        tone(BUZZER_PIN, 200, 200);
                        if (lives <= 0) {
                            state = STATE_GAMEOVER;
                            stateTimer = now;
                            tone(BUZZER_PIN, 150, 800);
                        }
                    }
                }
            }
        }

        display_clear();
        drawBasket((int)basketX, 110, (int)basketW, (int)basketH);
        
        for(int i=0; i<MAX_DROPS; i++) {
            if (drops[i].active) {
                if (drops[i].type == NORMAL) drawEgg((int)drops[i].x, (int)drops[i].y);
                else if (drops[i].type == GOLDEN) drawStar((int)drops[i].x, (int)drops[i].y);
                else if (drops[i].type == BOMB) drawBomb((int)drops[i].x, (int)drops[i].y);
            }
        }
        
        if (abs(wind) > 5.0f) {
            int wx = 32;
            int wy = 64;
            int dir = wind > 0 ? 5 : -5;
            fillRect(wx - abs(dir), wy, abs(dir)*2, 1);
            if (millis() % 200 < 100) fillRect(wx + dir, wy, abs(dir), 1);
        }
        
        drawText(2, 2, score);
        for(int i=0; i<lives; i++) {
            drawHeart(SCREEN_W - 10 - (i * 10), 2);
        }
        
        display_render();
        
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(5, 30, "GAME OVER");
        drawText(15, 50, "SCORE:");
        drawText(30, 65, score);
        display_render();
        
        if (now - stateTimer > 2000 && fire) {
            state = STATE_INTRO;
        }
    }
}

}
