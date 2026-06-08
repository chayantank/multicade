#include "barrel_main.h"
#include "barrel_display.h"
#include "barrel_input.h"
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 menuDisplay;

#define BUZZER_PIN 14

namespace Barrel {

const int SCREEN_W = 64;
const int SCREEN_H = 128;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_WIN
};

GameState state = STATE_INTRO;
unsigned long stateTimer = 0;

int score = 0;
int level = 1;
int lives = 3;

// --- Platforms ---
struct Platform {
    float x1, y1, x2, y2;
};

const int NUM_PLATFORMS = 5;
Platform platforms[NUM_PLATFORMS] = {
    {0, 115, 64, 115}, // Bottom ground (flat)
    {0, 85, 54, 90},   // P1 sloped down-right (gap on right)
    {10, 65, 64, 60},  // P2 sloped up-right (gap on left)
    {0, 35, 54, 40},   // P3 sloped down-right (gap on right)
    {20, 15, 64, 15}   // Top boss platform (flat)
};

// Get Y for a given X on a specific platform
float getPlatformY(int pIdx, float x) {
    if (x < platforms[pIdx].x1 || x > platforms[pIdx].x2) return -1; // off edge
    
    float t = (x - platforms[pIdx].x1) / (platforms[pIdx].x2 - platforms[pIdx].x1);
    return platforms[pIdx].y1 + t * (platforms[pIdx].y2 - platforms[pIdx].y1);
}

// --- Ladders ---
struct Ladder {
    float x;
    float topY;
    float botY;
};
const int NUM_LADDERS = 4;
Ladder ladders[NUM_LADDERS] = {
    {40, 87, 115},
    {20, 63, 87},
    {40, 37, 62},
    {30, 15, 37}
};

// --- Player ---
struct Player {
    float x, y;
    float vx, vy;
    bool onGround;
    bool onLadder;
    int animFrame;
    int pIdx; // current platform index
};
Player player;

// --- Barrels ---
struct Barr {
    float x, y;
    float vx, vy;
    bool active;
    bool falling;
    int pIdx;
    unsigned long scoreTimer;
};
const int MAX_BARRELS = 5;
Barr barrels[MAX_BARRELS];

unsigned long lastBarrelSpawn = 0;

void initLevel() {
    player.x = 10;
    player.y = 110;
    player.vx = 0;
    player.vy = 0;
    player.onGround = true;
    player.onLadder = false;
    player.pIdx = 0;
    
    for(int i=0; i<MAX_BARRELS; i++) {
        barrels[i].active = false;
    }
    
    lastBarrelSpawn = millis();
}

void resetGame() {
    score = 0;
    level = 1;
    lives = 3;
    initLevel();
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
    
    bool jump = input_jump();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(15, 40, "BARREL");
        drawText(10, 50, "CLIMBER");
        drawText(5, 70, "PRESS JUMP");
        display_render();
        
        if (jump) {
            resetGame();
            state = STATE_PLAYING;
        }
    } else if (state == STATE_PLAYING) {
        
        // --- Player Logic ---
        float speed = 30.0f;
        
        if (!player.onLadder) {
            if (input_left()) {
                player.vx = -speed;
                player.animFrame = (now / 150) % 2;
            } else if (input_right()) {
                player.vx = speed;
                player.animFrame = ((now / 150) % 2) + 2;
            } else {
                player.vx = 0;
            }
        }
        
        // Ladder climbing
        bool nearLadder = false;
        Ladder* curLadder = nullptr;
        for(int i=0; i<NUM_LADDERS; i++) {
            if (abs(player.x - ladders[i].x) < 4 && player.y >= ladders[i].topY - 2 && player.y <= ladders[i].botY + 2) {
                nearLadder = true;
                curLadder = &ladders[i];
                break;
            }
        }
        
        if (nearLadder) {
            if (!player.onLadder && (input_up() || input_down())) {
                player.onLadder = true;
                player.x = curLadder->x; // snap to ladder
                player.vx = 0;
                player.vy = 0;
            }
        } else {
            player.onLadder = false;
        }
        
        if (player.onLadder) {
            if (input_up()) {
                player.vy = -20.0f;
                player.animFrame = (now / 150) % 2 + 4; // climb anim
            } else if (input_down()) {
                player.vy = 20.0f;
                player.animFrame = (now / 150) % 2 + 4;
            } else {
                player.vy = 0;
            }
            
            // Check get off ladder
            if (player.y <= curLadder->topY) {
                player.y = curLadder->topY;
                player.onLadder = false;
                player.onGround = true;
            } else if (player.y >= curLadder->botY) {
                player.y = curLadder->botY;
                player.onLadder = false;
                player.onGround = true;
            }
            
            // Jump off ladder
            if (jump) {
                player.onLadder = false;
                player.vy = -40.0f;
                tone(BUZZER_PIN, 600, 50);
            }
            
            player.y += player.vy * dt;
            
        } else {
            // Normal Physics
            if (!player.onGround) {
                player.vy += 120.0f * dt; // Gravity
            } else {
                if (jump) {
                    player.vy = -65.0f;
                    player.onGround = false;
                    tone(BUZZER_PIN, 800, 50);
                }
            }
            
            player.x += player.vx * dt;
            player.y += player.vy * dt;
            
            // Screen boundaries
            if (player.x < 2) player.x = 2;
            if (player.x > SCREEN_W - 2) player.x = SCREEN_W - 2;
            
            // Collision with platforms
            player.onGround = false;
            if (player.vy >= 0) { // falling or flat
                for(int i=0; i<NUM_PLATFORMS; i++) {
                    float py = getPlatformY(i, player.x);
                    if (py != -1) {
                        // Check if we just crossed the platform
                        if (player.y >= py - 5 && player.y - player.vy * dt <= py + 2) {
                            player.y = py;
                            player.vy = 0;
                            player.onGround = true;
                            player.pIdx = i;
                            break;
                        }
                    }
                }
            }
        }
        
        // --- Win Condition ---
        if (player.y < 20) {
            score += 1000;
            level++;
            stateTimer = now;
            state = STATE_WIN;
            tone(BUZZER_PIN, 1500, 500);
        }
        
        // --- Barrel Logic ---
        int spawnRate = max(1000, 3000 - (level * 200));
        if (now - lastBarrelSpawn > spawnRate) {
            for(int i=0; i<MAX_BARRELS; i++) {
                if (!barrels[i].active) {
                    barrels[i].active = true;
                    barrels[i].x = 20; // Spawn near boss
                    barrels[i].y = 15;
                    barrels[i].vx = 30.0f + (level * 5.0f);
                    barrels[i].vy = 0;
                    barrels[i].falling = false;
                    barrels[i].pIdx = 4; // Top platform
                    barrels[i].scoreTimer = 0;
                    lastBarrelSpawn = now;
                    break;
                }
            }
        }
        
        bool isDead = false;
        
        for(int i=0; i<MAX_BARRELS; i++) {
            if (barrels[i].active) {
                if (barrels[i].falling) {
                    barrels[i].vy += 150.0f * dt;
                    barrels[i].y += barrels[i].vy * dt;
                    
                    // Check landing on next platform
                    for(int p=0; p<NUM_PLATFORMS; p++) {
                        float py = getPlatformY(p, barrels[i].x);
                        if (py != -1 && barrels[i].y >= py - 4 && barrels[i].y - barrels[i].vy * dt <= py + 2) {
                            barrels[i].falling = false;
                            barrels[i].y = py;
                            barrels[i].vy = 0;
                            barrels[i].pIdx = p;
                            // Reverse direction
                            if (barrels[i].x < 32) barrels[i].vx = abs(barrels[i].vx);
                            else barrels[i].vx = -abs(barrels[i].vx);
                            tone(BUZZER_PIN, 150, 20);
                            break;
                        }
                    }
                    
                    if (barrels[i].y > SCREEN_H + 10) barrels[i].active = false;
                    
                } else {
                    barrels[i].x += barrels[i].vx * dt;
                    
                    float py = getPlatformY(barrels[i].pIdx, barrels[i].x);
                    if (py == -1) {
                        // Fell off edge
                        barrels[i].falling = true;
                        barrels[i].vy = 10.0f; // slight push down
                    } else {
                        barrels[i].y = py;
                    }
                }
                
                // Collision with player
                if (abs(player.x - barrels[i].x) < 5 && abs((player.y - 4) - (barrels[i].y - 4)) < 6) {
                    isDead = true;
                }
                
                // Score for jumping over
                if (!barrels[i].falling && !player.onGround && !player.onLadder) {
                    if (abs(player.x - barrels[i].x) < 8 && player.y < barrels[i].y - 5 && barrels[i].scoreTimer == 0) {
                        score += 100;
                        barrels[i].scoreTimer = now; // Mark as scored
                        tone(BUZZER_PIN, 2000, 30);
                    }
                }
            }
        }
        
        if (isDead) {
            lives--;
            if (lives <= 0) {
                stateTimer = now;
                state = STATE_GAMEOVER;
                tone(BUZZER_PIN, 100, 1000);
            } else {
                initLevel();
                tone(BUZZER_PIN, 200, 500);
            }
        }
        
        // --- Render ---
        display_clear();
        
        // Platforms
        for(int i=0; i<NUM_PLATFORMS; i++) {
            drawLine(platforms[i].x1, platforms[i].y1, platforms[i].x2, platforms[i].y2, 1);
            drawLine(platforms[i].x1, platforms[i].y1+1, platforms[i].x2, platforms[i].y2+1, 1);
        }
        
        // Ladders
        for(int i=0; i<NUM_LADDERS; i++) {
            drawLine(ladders[i].x - 3, ladders[i].topY, ladders[i].x - 3, ladders[i].botY, 1);
            drawLine(ladders[i].x + 3, ladders[i].topY, ladders[i].x + 3, ladders[i].botY, 1);
            for(float y=ladders[i].topY + 2; y < ladders[i].botY; y += 4) {
                drawLine(ladders[i].x - 3, y, ladders[i].x + 3, y, 1);
            }
        }
        
        // Boss (Big Ape)
        fillRect(4, 5, 12, 10, 1);
        drawPixel(8, 7, 0); drawPixel(12, 7, 0); // Eyes
        if ((now / 500) % 2 == 0) {
            drawLine(2, 8, 4, 12, 1); // Arm down
            drawLine(16, 8, 14, 12, 1);
        } else {
            drawLine(2, 4, 4, 8, 1); // Arm up
            drawLine(16, 4, 14, 8, 1);
        }
        
        // Barrels
        for(int i=0; i<MAX_BARRELS; i++) {
            if (barrels[i].active) {
                int bx = (int)barrels[i].x;
                int by = (int)barrels[i].y;
                drawCircle(bx, by - 4, 4, 1);
                // Roll animation
                int r = (int)(barrels[i].x) % 8;
                if (r < 4) {
                    drawLine(bx-2, by-6, bx+2, by-2, 1);
                    drawLine(bx+2, by-6, bx-2, by-2, 1);
                } else {
                    drawLine(bx, by-8, bx, by, 1);
                    drawLine(bx-4, by-4, bx+4, by-4, 1);
                }
            }
        }
        
        // Player
        int px = (int)player.x;
        int py = (int)player.y;
        
        // Head
        fillRect(px-2, py-8, 4, 4, 1);
        
        // Body / Legs based on anim
        if (player.onLadder) {
            drawLine(px, py-4, px, py-2, 1);
            if (player.animFrame == 4) {
                drawLine(px, py-2, px-2, py, 1);
                drawLine(px, py-2, px+1, py, 1);
            } else {
                drawLine(px, py-2, px-1, py, 1);
                drawLine(px, py-2, px+2, py, 1);
            }
        } else {
            drawLine(px, py-4, px, py-2, 1); // Body
            if (player.vx != 0) {
                if (player.animFrame % 2 == 0) {
                    drawLine(px, py-2, px-2, py, 1); // Legs split
                    drawLine(px, py-2, px+2, py, 1);
                } else {
                    drawLine(px, py-2, px, py, 1); // Legs together
                }
            } else {
                drawLine(px, py-2, px-2, py, 1); // Stand
                drawLine(px, py-2, px+2, py, 1);
            }
        }
        
        // HUD
        fillRect(30, 0, 34, 12, 0); // Clear top right
        menuDisplay.setTextColor(WHITE);
        menuDisplay.setTextSize(1);
        menuDisplay.setCursor(30, 0); menuDisplay.print(score);
        
        for(int i=0; i<lives; i++) {
            drawPixel(60 - (i*4), 9, 1);
            drawPixel(59 - (i*4), 10, 1);
            drawPixel(61 - (i*4), 10, 1);
            drawPixel(60 - (i*4), 11, 1);
        }
        
        display_render();
        
    } else if (state == STATE_WIN) {
        display_clear();
        drawText(10, 40, "LEVEL");
        drawText(45, 40, level - 1);
        drawText(10, 50, "CLEARED!");
        display_render();
        if (now - stateTimer > 2000) {
            initLevel();
            state = STATE_PLAYING;
        }
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(5, 40, "GAME OVER");
        drawText(5, 60, "SCORE:");
        drawText(5, 70, score);
        display_render();
        
        if (jump && now - stateTimer > 1000) {
            state = STATE_INTRO;
        }
    }
}

}
