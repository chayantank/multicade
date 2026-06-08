#include "invaders_main.h"
#include "invaders_display.h"
#include "invaders_input.h"
#include "invaders_sprites.h"

namespace Invaders {

// Game Constants
const int SCREEN_W = 64;
const int SCREEN_H = 128;
const int ALIEN_ROWS = 5;
const int ALIEN_COLS = 4;
const int MAX_BULLETS = 6; // Increased to 6 for double wing fire
const int MAX_ALIEN_BULLETS = 3;

// Game States
enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_VICTORY
};

GameState state = STATE_INTRO;
int level = 1;

// Entities
struct Player {
    float x;
    float y;
    int score;
    int lives;
    unsigned long lastFireTime;
} player;

struct Bullet {
    float x;
    float y;
    bool active;
};

Bullet bullets[MAX_BULLETS];
Bullet alienBullets[MAX_ALIEN_BULLETS];

struct Alien {
    float x;
    float y;
    bool active;
    uint8_t type;
};

Alien aliens[ALIEN_ROWS * ALIEN_COLS];

float alienDirX = 1.0f;
float alienSpeed = 0.5f;
float alienDrop = 0.0f;
unsigned long lastAlienMove = 0;
unsigned long lastFrameTime = 0;

void initLevel() {
    // Init player
    player.x = SCREEN_W / 2 - SHIP_W / 2;
    player.y = SCREEN_H - SHIP_H - 2;
    player.lastFireTime = 0;

    // Init bullets
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
    for (int i = 0; i < MAX_ALIEN_BULLETS; i++) alienBullets[i].active = false;

    // Init aliens
    int startX = 4;
    int startY = 16; // Give some room at the top for HUD
    int spacingX = 14;
    int spacingY = 10;

    for (int r = 0; r < ALIEN_ROWS; r++) {
        for (int c = 0; c < ALIEN_COLS; c++) {
            int idx = r * ALIEN_COLS + c;
            aliens[idx].x = startX + c * spacingX;
            aliens[idx].y = startY + r * spacingY;
            aliens[idx].active = true;
            aliens[idx].type = r % 2; // alternate types
        }
    }

    alienDirX = 1.0f;
    alienSpeed = 20.0f + (level - 1) * 8.0f; // Increase speed per level
}

void setup() {
    display_setup();
    input_setup();
    
    player.score = 0;
    player.lives = 3;
    state = STATE_INTRO;
}

bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

void loopPlaying(float dt) {
    // Player Input
    if (input_left() && player.x > 0) player.x -= 60.0f * dt;
    if (input_right() && player.x < SCREEN_W - SHIP_W) player.x += 60.0f * dt;

    if (input_fire() && millis() - player.lastFireTime > 300) {
        int spawned = 0;
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) {
                bullets[i].active = true;
                if (spawned == 0) {
                    // Left wing
                    bullets[i].x = player.x + 1;
                    bullets[i].y = player.y;
                    spawned++;
                } else {
                    // Right wing
                    bullets[i].x = player.x + SHIP_W - 2;
                    bullets[i].y = player.y;
                    player.lastFireTime = millis();
                    tone(14, 800, 50); // Shoot sound
                    break;
                }
            }
        }
    }

    // Update Player Bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y -= 100.0f * dt;
            if (bullets[i].y < 0) bullets[i].active = false;
        }
    }

    // Update Alien Bullets
    for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
        if (alienBullets[i].active) {
            alienBullets[i].y += 50.0f * dt;
            if (alienBullets[i].y > SCREEN_H) alienBullets[i].active = false;
            
            // Check collision with player
            if (checkCollision(alienBullets[i].x, alienBullets[i].y, 1, 3, player.x, player.y, SHIP_W, SHIP_H)) {
                alienBullets[i].active = false;
                player.lives--;
                tone(14, 200, 300); // Player hit sound
                if (player.lives <= 0) {
                    state = STATE_GAME_OVER;
                    tone(14, 100, 500); // Game over sound
                }
            }
        }
    }

    // Update Aliens
    bool hitEdge = false;
    int activeAliens = 0;
    
    for (int i = 0; i < ALIEN_ROWS * ALIEN_COLS; i++) {
        if (aliens[i].active) {
            activeAliens++;
            aliens[i].x += alienDirX * alienSpeed * dt;
            if (aliens[i].x <= 0 || aliens[i].x >= SCREEN_W - ALIEN_W) {
                hitEdge = true;
            }
        }
    }

    if (activeAliens == 0) {
        state = STATE_VICTORY;
        tone(14, 1200, 400); // Victory sound
    }

    if (hitEdge) {
        alienDirX *= -1;
        alienSpeed += 2.0f; // Speed up less aggressively
        for (int i = 0; i < ALIEN_ROWS * ALIEN_COLS; i++) {
            if (aliens[i].active) {
                aliens[i].x += alienDirX * alienSpeed * dt; // Move out of edge
                aliens[i].y += 2.0f; // Drop down less drastically for portrait
                if (aliens[i].y >= player.y - ALIEN_H) {
                    state = STATE_GAME_OVER; // Aliens reached player
                    tone(14, 100, 500); // Game over sound
                }
            }
        }
    }

    // Alien Shooting
    if (activeAliens > 0 && random(0, 100) < (2 + level)) { // More aggressive firing per level
        for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
            if (!alienBullets[i].active) {
                // Pick a random active alien
                int r = random(0, ALIEN_ROWS * ALIEN_COLS);
                while (!aliens[r].active) {
                    r = random(0, ALIEN_ROWS * ALIEN_COLS);
                }
                alienBullets[i].active = true;
                alienBullets[i].x = aliens[r].x + ALIEN_W / 2;
                alienBullets[i].y = aliens[r].y + ALIEN_H;
                break;
            }
        }
    }

    // Collisions Player Bullet <-> Aliens
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            for (int j = 0; j < ALIEN_ROWS * ALIEN_COLS; j++) {
                if (aliens[j].active) {
                    if (checkCollision(bullets[i].x, bullets[i].y, 1, 3, aliens[j].x, aliens[j].y, ALIEN_W, ALIEN_H)) {
                        bullets[i].active = false;
                        aliens[j].active = false;
                        player.score += 10;
                        tone(14, 400, 50); // Alien destroyed sound
                        break;
                    }
                }
            }
        }
    }

    // Drawing
    display_clear();

    // Draw Player
    drawSprite((int)player.x, (int)player.y, bmp_player, SHIP_W, SHIP_H);

    // Draw Bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) fillRect((int)bullets[i].x, (int)bullets[i].y, 1, 3, 1);
    }
    for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
        if (alienBullets[i].active) fillRect((int)alienBullets[i].x, (int)alienBullets[i].y, 1, 3, 1);
    }

    // Draw Aliens
    for (int i = 0; i < ALIEN_ROWS * ALIEN_COLS; i++) {
        if (aliens[i].active) {
            const uint8_t* bmp = aliens[i].type == 0 ? bmp_alien_a : bmp_alien_b;
            drawSprite((int)aliens[i].x, (int)aliens[i].y, bmp, ALIEN_W, ALIEN_H);
        }
    }

    // Draw Score and Lives
    drawText(0, 0, "S:");
    drawText(12, 0, player.score);
    drawText(36, 0, "L:");
    drawText(48, 0, player.lives);

    display_render();
}

void loop() {
    unsigned long now = millis();
    float dt = (now - lastFrameTime) / 1000.0f;
    lastFrameTime = now;
    
    // Cap dt to prevent massive jumps if lagging
    if (dt > 0.1f) dt = 0.1f;

    // We process input here just to check for 3-sec hold
    // Actual gameplay input is handled in loopPlaying
    bool fire_pressed = input_fire();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(16, 20, "SPACE");
        drawText(8, 30, "INVADERS");
        drawText(2, 50, "PRESS FIRE");
        display_render();
        
        if (fire_pressed) {
            level = 1;
            initLevel();
            state = STATE_PLAYING;
            lastFrameTime = millis();
        }
    } else if (state == STATE_PLAYING) {
        loopPlaying(dt);
    } else if (state == STATE_GAME_OVER) {
        display_clear();
        drawText(5, 20, "GAME OVER");
        drawText(0, 40, "Score:");
        drawText(40, 40, player.score);
        display_render();
        
        if (fire_pressed) {
            player.score = 0;
            player.lives = 3;
            state = STATE_INTRO;
        }
    } else if (state == STATE_VICTORY) {
        display_clear();
        drawText(20, 20, "WAVE");
        drawText(8, 30, "CLEARED!");
        drawText(2, 50, "PRESS FIRE");
        display_render();
        
        if (fire_pressed) {
            level++;
            initLevel();
            state = STATE_PLAYING;
            lastFrameTime = millis();
        }
    }
}

}
