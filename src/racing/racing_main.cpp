#include "racing_main.h"
#include "racing_display.h"
#include "racing_input.h"
#include "racing_sprites.h"

#define BUZZER_PIN 14

namespace Racing {

const int SCREEN_W = 64;
const int SCREEN_H = 128;
const int MAX_ENEMIES = 4;

// Lane positions (X coordinates for the car sprites)
const int laneX[3] = {5, 26, 47};

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_CRASH,
    STATE_GAME_OVER
};

GameState state = STATE_INTRO;

struct Player {
    int lane; // 0, 1, 2
    float y;
} player;

struct Enemy {
    int lane;
    float y;
    bool active;
};

Enemy enemies[MAX_ENEMIES];

int score = 0;
int level = 1;
int lives = 3;
float enemySpeed = 40.0f;
float roadOffset = 0.0f;

unsigned long lastFrameTime = 0;
unsigned long stateTimer = 0;
unsigned long lastEnemySpawn = 0;

// Input debouncing
bool lastLeft = false;
bool lastRight = false;

void resetLevel() {
    player.lane = 1;
    player.y = SCREEN_H - CAR_H - 4;
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
    roadOffset = 0.0f;
    enemySpeed = 50.0f + (level - 1) * 10.0f;
    lastEnemySpawn = millis();
}

void setup() {
    display_setup();
    input_setup();
    score = 0;
    level = 1;
    lives = 3;
    state = STATE_INTRO;
}

bool checkCollision(int l1, float y1, int l2, float y2) {
    if (l1 != l2) return false;
    // Cars are in the same lane, check Y collision (height is CAR_H)
    // Hitbox slightly smaller than full sprite
    int hitboxH = CAR_H - 2;
    if (y1 < y2 + hitboxH && y1 + hitboxH > y2) return true;
    return false;
}

void loopPlaying(float dt) {
    // Process input
    bool currentLeft = input_left();
    bool currentRight = input_right();

    if (currentLeft && !lastLeft && player.lane > 0) {
        player.lane--;
        tone(BUZZER_PIN, 800, 20); // Switch lane sound
    }
    if (currentRight && !lastRight && player.lane < 2) {
        player.lane++;
        tone(BUZZER_PIN, 800, 20); // Switch lane sound
    }

    lastLeft = currentLeft;
    lastRight = currentRight;

    // Move road
    roadOffset += enemySpeed * dt;
    if (roadOffset >= 16.0f) roadOffset -= 16.0f;

    // Spawn enemies
    // Spawn rate scales with level
    float spawnDelay = 1500.0f / (1.0f + (level - 1) * 0.2f);
    if (millis() - lastEnemySpawn > spawnDelay) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].active) {
                enemies[i].active = true;
                enemies[i].lane = random(0, 3);
                enemies[i].y = -CAR_H; // Start just above screen
                lastEnemySpawn = millis();
                break;
            }
        }
    }

    // Update enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            enemies[i].y += enemySpeed * dt;
            
            // Check collision
            if (checkCollision(player.lane, player.y, enemies[i].lane, enemies[i].y)) {
                state = STATE_CRASH;
                stateTimer = millis();
                tone(BUZZER_PIN, 150, 500); // Crash sound
                return;
            }

            // Passed enemy
            if (enemies[i].y > SCREEN_H) {
                enemies[i].active = false;
                score++;
                tone(BUZZER_PIN, 1200, 30); // Pass sound
                
                // Level up every 10 points
                if (score % 10 == 0) {
                    level++;
                    enemySpeed = 50.0f + (level - 1) * 10.0f;
                    tone(BUZZER_PIN, 1500, 200); // Level up sound
                }
            }
        }
    }

    // Drawing
    display_clear();

    // Draw Road Lines
    for (int y = (int)roadOffset - 16; y < SCREEN_H; y += 16) {
        drawLine(21, y, 21, y + 8, WHITE);
        drawLine(42, y, 42, y + 8, WHITE);
    }
    
    // Draw Border Lines
    drawLine(0, 0, 0, SCREEN_H, WHITE);
    drawLine(63, 0, 63, SCREEN_H, WHITE);

    // Draw Player
    drawSprite(laneX[player.lane], (int)player.y, bmp_player_car, CAR_W, CAR_H);

    // Draw Enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            drawSprite(laneX[enemies[i].lane], (int)enemies[i].y, bmp_enemy_car, CAR_W, CAR_H);
        }
    }

    // Draw Score and Lives
    fillRect(0, 0, 64, 9, 0); // Black background for top HUD
    drawText(2, 1, "S:");
    drawText(14, 1, score);
    
    drawText(40, 1, "L:");
    drawText(52, 1, lives);

    display_render();
}

void loop() {
    unsigned long now = millis();
    float dt = (now - lastFrameTime) / 1000.0f;
    lastFrameTime = now;
    if (dt > 0.1f) dt = 0.1f;

    bool fire_pressed = input_fire();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(14, 20, "RACING");
        drawText(8, 60, "PRESS FIRE");
        display_render();

        if (fire_pressed) {
            level = 1;
            score = 0;
            lives = 3;
            resetLevel();
            state = STATE_PLAYING;
            lastFrameTime = millis();
        }
    } else if (state == STATE_PLAYING) {
        loopPlaying(dt);
    } else if (state == STATE_CRASH) {
        display_clear();
        drawText(18, 40, "CRASH!");
        display_render();

        if (millis() - stateTimer > 1500) {
            lives--;
            if (lives <= 0) {
                state = STATE_GAME_OVER;
                tone(BUZZER_PIN, 100, 600);
            } else {
                resetLevel();
                state = STATE_PLAYING;
                lastFrameTime = millis();
            }
        }
    } else if (state == STATE_GAME_OVER) {
        display_clear();
        drawText(5, 30, "GAME OVER");
        drawText(2, 60, "SCORE:");
        drawText(40, 60, score);
        display_render();

        if (fire_pressed) {
            state = STATE_INTRO;
        }
    }
}

}
