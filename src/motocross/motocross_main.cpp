#include "motocross_main.h"
#include "motocross_display.h"
#include "motocross_input.h"

#define BUZZER_PIN 14

namespace MotoCross {

const int SCREEN_W = 128;
const int SCREEN_H = 64;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;
unsigned long stateTimer = 0;

int score = 0;
float distance = 0;
float temp = 0;
const float MAX_TEMP = 100.0f;
bool overheated = false;
unsigned long overheatTimer = 0;

int lane = 2; // 0 to 3
float pY = 40.0f; // lane 0=25, 1=35, 2=45, 3=55
float pZ = 0.0f; // Height (jumping)
float vZ = 0.0f;
bool inAir = false;

float speed = 0;

struct Obstacle {
    float x;
    int lane;
    int type; // 1=Mud, 2=Ramp
    bool active;
};

const int MAX_OBS = 5;
Obstacle obs[MAX_OBS];

void spawnObstacle(int i) {
    obs[i].active = true;
    obs[i].x = SCREEN_W + random(0, 100);
    obs[i].lane = random(0, 4);
    obs[i].type = random(1, 3);
    
    // Make sure it doesn't overlap exactly
    for(int j=0; j<MAX_OBS; j++) {
        if (i != j && obs[j].active && obs[j].lane == obs[i].lane && abs(obs[j].x - obs[i].x) < 30) {
            obs[i].x += 30; // Push back
        }
    }
}

void resetGame() {
    score = 0;
    distance = 0;
    temp = 0;
    overheated = false;
    lane = 2;
    pY = 45.0f;
    pZ = 0;
    vZ = 0;
    inAir = false;
    
    for(int i=0; i<MAX_OBS; i++) {
        obs[i].active = false;
    }
    for(int i=0; i<MAX_OBS; i++) {
        spawnObstacle(i);
        obs[i].x += i * 40; // Initial spread
    }
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

unsigned long lastTime = 0;
unsigned long lastLaneChange = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    bool boost = input_boost();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(35, 20, "MOTOCROSS");
        drawText(20, 40, "HOLD BTN TO BOOST");
        display_render();
        
        if (boost) {
            resetGame();
            state = STATE_PLAYING;
        }
    } else if (state == STATE_PLAYING) {
        
        // --- Speed & Temp ---
        bool inMud = false;
        
        // Check ground collisions
        if (!inAir && !overheated) {
            for(int i=0; i<MAX_OBS; i++) {
                if (obs[i].active && obs[i].lane == lane) {
                    if (obs[i].x < 20 && obs[i].x + 12 > 10) {
                        if (obs[i].type == 1) {
                            inMud = true;
                        } else if (obs[i].type == 2) {
                            // Hit Ramp
                            inAir = true;
                            vZ = 40.0f; // Jump velocity
                            tone(BUZZER_PIN, 1200, 100);
                        }
                    }
                }
            }
        }
        
        if (overheated) {
            speed = 0;
            if (now - overheatTimer > 2000) {
                overheated = false;
                temp = 0;
            }
        } else if (inMud) {
            speed = 20.0f; // Slow
            if (boost) temp += 20.0f * dt; // Still heat up if holding boost in mud
            else temp -= 10.0f * dt;
        } else if (boost) {
            speed = 80.0f; // Fast
            temp += 30.0f * dt; // Heat up
            if (temp >= MAX_TEMP) {
                overheated = true;
                overheatTimer = now;
                tone(BUZZER_PIN, 100, 1000); // Stall sound
            }
        } else {
            speed = 40.0f; // Normal
            temp -= 15.0f * dt; // Cool down
        }
        
        if (temp < 0) temp = 0;
        
        // Engine sound
        if (!overheated) {
            if (now % 100 < 50) tone(BUZZER_PIN, 50 + speed, 20);
        }
        
        // --- Movement ---
        distance += speed * dt;
        score = (int)(distance / 10);
        
        if (!inAir && !overheated && speed > 0) {
            if (now - lastLaneChange > 150) {
                if (input_up() && lane > 0) {
                    lane--;
                    lastLaneChange = now;
                } else if (input_down() && lane < 3) {
                    lane++;
                    lastLaneChange = now;
                }
            }
        }
        
        // Target Y for lane
        float targetY = 25.0f + lane * 10.0f;
        pY += (targetY - pY) * 10.0f * dt; // Smooth lane change
        
        // Jump Physics
        if (inAir) {
            pZ += vZ * dt;
            vZ -= 100.0f * dt; // Gravity
            if (pZ <= 0) {
                pZ = 0;
                inAir = false;
                tone(BUZZER_PIN, 400, 50); // Land sound
            }
        }
        
        // --- Obstacles ---
        for(int i=0; i<MAX_OBS; i++) {
            if (obs[i].active) {
                obs[i].x -= speed * dt;
                if (obs[i].x < -20) {
                    spawnObstacle(i); // Recycle
                }
            }
        }
        
        // Level up speed modifier? (Distance based)
        // Just keep endless for now.
        
        // --- Render ---
        display_clear();
        
        // Draw Lanes
        for(int i=0; i<=4; i++) {
            int ly = 20 + i*10;
            if (i == 0 || i == 4) {
                drawLine(0, ly, SCREEN_W, ly, 1); // Solid border
            } else {
                // Dashed lines
                int offset = -((int)distance % 10);
                for(int x = offset; x < SCREEN_W; x += 10) {
                    drawLine(x, ly, x+4, ly, 1);
                }
            }
        }
        
        // Draw Obstacles
        for(int i=0; i<MAX_OBS; i++) {
            if (obs[i].active) {
                int ox = (int)obs[i].x;
                int oy = 25 + obs[i].lane * 10;
                
                if (obs[i].type == 1) {
                    // Mud (jagged puddle)
                    drawPixel(ox+2, oy-3, 1);
                    drawLine(ox, oy-2, ox+6, oy-2, 1);
                    drawLine(ox-2, oy-1, ox+8, oy-1, 1);
                    drawLine(ox-1, oy, ox+7, oy, 1);
                } else if (obs[i].type == 2) {
                    // Ramp
                    drawLine(ox, oy, ox+10, oy-4, 1); // slope
                    drawLine(ox+10, oy-4, ox+10, oy, 1); // back
                }
            }
        }
        
        // Draw Player
        int drawX = 10;
        int drawY = (int)pY - (int)pZ;
        
        // Bike body
        drawLine(drawX, drawY-2, drawX+8, drawY-2, 1);
        // Wheels
        drawPixel(drawX, drawY, 1); drawPixel(drawX+8, drawY, 1);
        // Rider
        drawLine(drawX+4, drawY-2, drawX+6, drawY-5, 1); // body leaning
        drawPixel(drawX+7, drawY-6, 1); // head
        
        if (overheated) {
            // Smoke
            if (random(0,2)) drawPixel(drawX-2, drawY-4, 1);
            if (random(0,2)) drawPixel(drawX-4, drawY-6, 1);
        }
        
        // HUD
        fillRect(0, 0, SCREEN_W, 10, 0); // Clear top
        drawText(0, 1, "SCORE:"); drawText(35, 1, score);
        
        // Temp gauge
        drawText(70, 1, "TMP");
        drawRect(90, 1, 32, 6, 1);
        int barW = (int)((temp / MAX_TEMP) * 30);
        if (barW > 0) {
            if (overheated) {
                if ((now / 100) % 2 == 0) fillRect(91, 2, barW, 4, 1); // blink
            } else {
                fillRect(91, 2, barW, 4, 1);
            }
        }
        
        display_render();
        
        // Game Over condition?
        // Let's say if you hit 500 score, you win.
        if (score >= 500) {
            state = STATE_GAMEOVER;
            stateTimer = now;
            tone(BUZZER_PIN, 2000, 1000);
        }
        
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(30, 20, "YOU WIN!");
        drawText(20, 40, "SCORE:");
        drawText(60, 40, score);
        display_render();
        if (boost && now - stateTimer > 1000) {
            state = STATE_INTRO;
        }
    }
}

}
