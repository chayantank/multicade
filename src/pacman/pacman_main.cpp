#include "pacman_main.h"
#include "pacman_display.h"
#include "pacman_input.h"
#include "pacman_sprites.h"
#include "pacman_map.h"

namespace Pacman {

// Game Constants
const int TILE_SIZE = 8;
const int SCREEN_W = 64;
const int SCREEN_H = 128;
const int MAX_GHOSTS = 3;

// Directions
enum Direction { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_NONE };

// Game States
enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_DYING,
    STATE_GAME_OVER,
    STATE_VICTORY
};

GameState state = STATE_INTRO;
int level = 1;

// Dynamic map to track eaten dots
uint8_t current_map[MAP_H][MAP_W];
int total_dots = 0;
int dots_eaten = 0;

struct Entity {
    float x;
    float y;
    int gridX;
    int gridY;
    Direction dir;
    Direction nextDir;
    float speed;
};

Entity player;

struct Ghost {
    Entity e;
    bool scared;
    unsigned long scaredStartTime;
};

Ghost ghosts[MAX_GHOSTS];

int score = 0;
int lives = 3;
unsigned long lastFrameTime = 0;

void resetEntities() {
    // Player start pos (grid 3, 11)
    player.gridX = 3;
    player.gridY = 11;
    player.x = player.gridX * TILE_SIZE;
    player.y = player.gridY * TILE_SIZE;
    player.dir = DIR_NONE; // Start still until user gives input
    player.nextDir = DIR_NONE;
    player.speed = 24.0f + (level - 1) * 2.0f;

    // Ghosts start pos (grid 3 and 4, 3) outside ghost box, immediately in play
    for (int i = 0; i < MAX_GHOSTS; i++) {
        ghosts[i].e.gridX = 3 + (i % 2); // 3, 4, 3
        ghosts[i].e.gridY = 3;
        ghosts[i].e.x = ghosts[i].e.gridX * TILE_SIZE;
        ghosts[i].e.y = ghosts[i].e.gridY * TILE_SIZE;
        ghosts[i].e.dir = DIR_UP;
        ghosts[i].e.nextDir = DIR_UP;
        ghosts[i].e.speed = 18.0f + (level - 1) * 3.0f;
        ghosts[i].scared = false;
    }
}

void initLevel() {
    total_dots = 0;
    dots_eaten = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            current_map[y][x] = pgm_read_byte(&default_map[y][x]);
            if (current_map[y][x] == D || current_map[y][x] == P) {
                total_dots++;
            }
        }
    }
    resetEntities();
}

void setup() {
    display_setup();
    input_setup();
    score = 0;
    lives = 3;
    state = STATE_INTRO;
}

bool isWalkable(int gx, int gy, bool isGhost) {
    if (gx < 0 || gx >= MAP_W || gy < 0 || gy >= MAP_H) return false;
    uint8_t tile = current_map[gy][gx];
    if (tile == W) return false;
    if (tile == G && !isGhost) return false; // Player can't enter ghost door
    return true;
}

void onReachIntersection(Entity& ent, bool isGhost) {
    if (isGhost) {
        // Ghost AI decision making
        Ghost* g = nullptr;
        for(int i=0; i<MAX_GHOSTS; i++) {
            if(&ghosts[i].e == &ent) g = &ghosts[i];
        }
        if (g) {
            int validDirs[4];
            int count = 0;
            Direction options[4] = {DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT};
            Direction backwards = DIR_NONE;
            if (ent.dir == DIR_UP) backwards = DIR_DOWN;
            if (ent.dir == DIR_DOWN) backwards = DIR_UP;
            if (ent.dir == DIR_LEFT) backwards = DIR_RIGHT;
            if (ent.dir == DIR_RIGHT) backwards = DIR_LEFT;

            for (int i=0; i<4; i++) {
                Direction d = options[i];
                if (d == backwards && !g->scared) continue; 
                int nx = ent.gridX; 
                int ny = ent.gridY;
                if (d == DIR_UP) ny--;
                else if (d == DIR_DOWN) ny++;
                else if (d == DIR_LEFT) nx--;
                else if (d == DIR_RIGHT) nx++;
                
                if (nx >= 0 && nx < MAP_W && isWalkable(nx, ny, true)) {
                    validDirs[count++] = d;
                }
            }
            if (count > 0) {
                ent.nextDir = (Direction)validDirs[random(0, count)];
            }
        }
    }

    // Try to turn to nextDir
    if (ent.nextDir != DIR_NONE && ent.nextDir != ent.dir) {
        int nx = ent.gridX; 
        int ny = ent.gridY;
        if (ent.nextDir == DIR_UP) ny--;
        else if (ent.nextDir == DIR_DOWN) ny++;
        else if (ent.nextDir == DIR_LEFT) nx--;
        else if (ent.nextDir == DIR_RIGHT) nx++;
        if (isWalkable(nx, ny, isGhost)) {
            ent.dir = ent.nextDir;
        }
    }
}

void moveEntity(Entity& ent, float dt, bool isGhost) {
    if (ent.dir == DIR_NONE) {
        if (ent.nextDir != DIR_NONE) {
            int nx = ent.gridX; 
            int ny = ent.gridY;
            if (ent.nextDir == DIR_UP) ny--;
            else if (ent.nextDir == DIR_DOWN) ny++;
            else if (ent.nextDir == DIR_LEFT) nx--;
            else if (ent.nextDir == DIR_RIGHT) nx++;
            if (isWalkable(nx, ny, isGhost)) {
                ent.dir = ent.nextDir;
            }
        }
        return;
    }

    float moveDist = ent.speed * dt;
    float prevX = ent.x;
    float prevY = ent.y;

    if (ent.dir == DIR_UP) ent.y -= moveDist;
    else if (ent.dir == DIR_DOWN) ent.y += moveDist;
    else if (ent.dir == DIR_LEFT) ent.x -= moveDist;
    else if (ent.dir == DIR_RIGHT) ent.x += moveDist;

    int targetGridX = ent.gridX;
    int targetGridY = ent.gridY;
    if (ent.dir == DIR_UP) targetGridY--;
    else if (ent.dir == DIR_DOWN) targetGridY++;
    else if (ent.dir == DIR_LEFT) targetGridX--;
    else if (ent.dir == DIR_RIGHT) targetGridX++;

    float targetPx = targetGridX * TILE_SIZE;
    float targetPy = targetGridY * TILE_SIZE;

    bool crossed = false;
    if (ent.dir == DIR_UP && ent.y <= targetPy) crossed = true;
    else if (ent.dir == DIR_DOWN && ent.y >= targetPy) crossed = true;
    else if (ent.dir == DIR_LEFT && ent.x <= targetPx) crossed = true;
    else if (ent.dir == DIR_RIGHT && ent.x >= targetPx) crossed = true;

    if (crossed) {
        ent.gridX = targetGridX;
        ent.gridY = targetGridY;
        
        if (ent.gridX < 0) {
            ent.gridX = MAP_W - 1;
            ent.x = ent.gridX * TILE_SIZE;
            return;
        } else if (ent.gridX >= MAP_W) {
            ent.gridX = 0;
            ent.x = 0;
            return;
        }

        ent.x = targetPx;
        ent.y = targetPy;

        onReachIntersection(ent, isGhost);

        int nx = ent.gridX; 
        int ny = ent.gridY;
        if (ent.dir == DIR_UP) ny--;
        else if (ent.dir == DIR_DOWN) ny++;
        else if (ent.dir == DIR_LEFT) nx--;
        else if (ent.dir == DIR_RIGHT) nx++;
        
        if (!isWalkable(nx, ny, isGhost)) {
            ent.dir = DIR_NONE;
        } else {
            float distToTarget = (ent.dir == DIR_LEFT || ent.dir == DIR_RIGHT) ? abs(prevX - targetPx) : abs(prevY - targetPy);
            float remainder = moveDist - distToTarget;
            if (remainder > 0) {
                if (ent.dir == DIR_UP) ent.y -= remainder;
                else if (ent.dir == DIR_DOWN) ent.y += remainder;
                else if (ent.dir == DIR_LEFT) ent.x -= remainder;
                else if (ent.dir == DIR_RIGHT) ent.x += remainder;
            }
        }
    }
}

bool checkCollisionRect(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

void loopPlaying(float dt) {
    // Input
    if (input_up()) player.nextDir = DIR_UP;
    if (input_down()) player.nextDir = DIR_DOWN;
    if (input_left()) player.nextDir = DIR_LEFT;
    if (input_right()) player.nextDir = DIR_RIGHT;

    moveEntity(player, dt, false);

    // Eat dots
    uint8_t tile = current_map[player.gridY][player.gridX];
    if (tile == D) {
        current_map[player.gridY][player.gridX] = E;
        score += 10;
        tone(14, 600, 20); // Eat dot
        dots_eaten++;
    } else if (tile == P) {
        current_map[player.gridY][player.gridX] = E;
        score += 50;
        tone(14, 1000, 50); // Eat power pellet
        dots_eaten++;
        for (int i=0; i<MAX_GHOSTS; i++) {
            ghosts[i].scared = true;
            ghosts[i].scaredStartTime = millis();
            ghosts[i].e.speed = 12.0f + (level - 1) * 1.5f; // slower when scared
            
            // Force reverse direction
            if (ghosts[i].e.dir == DIR_UP) ghosts[i].e.nextDir = DIR_DOWN;
            else if (ghosts[i].e.dir == DIR_DOWN) ghosts[i].e.nextDir = DIR_UP;
            else if (ghosts[i].e.dir == DIR_LEFT) ghosts[i].e.nextDir = DIR_RIGHT;
            else if (ghosts[i].e.dir == DIR_RIGHT) ghosts[i].e.nextDir = DIR_LEFT;
        }
    }

    if (dots_eaten >= total_dots) {
        state = STATE_VICTORY;
        tone(14, 1200, 400); // Victory
    }

    // Update ghosts
    for (int i=0; i<MAX_GHOSTS; i++) {
        moveEntity(ghosts[i].e, dt, true);

        // Check scared timeout
        int scaredDuration = 8000 - (level - 1) * 1000;
        if (scaredDuration < 2000) scaredDuration = 2000;
        if (ghosts[i].scared && millis() - ghosts[i].scaredStartTime > scaredDuration) {
            ghosts[i].scared = false;
            ghosts[i].e.speed = 18.0f + (level - 1) * 3.0f;
        }

        // Collision with player (Tile size is 8, use 6 for generous hitbox)
        if (checkCollisionRect(player.x+1, player.y+1, 6, 6, 
                               ghosts[i].e.x+1, ghosts[i].e.y+1, 6, 6)) {
            if (ghosts[i].scared) {
                // Eat ghost
                score += 200;
                tone(14, 1500, 100); // Eat ghost
                ghosts[i].scared = false;
                ghosts[i].e.gridX = 3 + (i % 2);
                ghosts[i].e.gridY = 3;
                ghosts[i].e.x = ghosts[i].e.gridX * TILE_SIZE;
                ghosts[i].e.y = ghosts[i].e.gridY * TILE_SIZE;
                ghosts[i].e.speed = 18.0f + (level - 1) * 3.0f;
            } else {
                // Player dies
                lives--;
                if (lives <= 0) {
                    state = STATE_GAME_OVER;
                    tone(14, 100, 500); // Game Over
                } else {
                    tone(14, 300, 300); // Lose life
                    resetEntities();
                    // pause briefly
                    delay(1000);
                    lastFrameTime = millis();
                }
            }
        }
    }

    // Drawing
    display_clear();

    // The map is 8x15 tiles (64x120 pixels). Screen is 64x128.
    // X offset = 0, Y offset = 0.
    int offX = 0;
    int offY = 0;

    // Draw Map
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            uint8_t t = current_map[y][x];
            int px = offX + x * TILE_SIZE;
            int py = offY + y * TILE_SIZE;
            if (t == W) {
                // Draw a simple 8x8 hollow box for walls
                display.drawRect(px, py, TILE_SIZE, TILE_SIZE, 1);
            } else if (t == D) {
                // Draw dot
                fillRect(px + 3, py + 3, 2, 2, 1);
            } else if (t == P) {
                // Draw power pellet
                fillRect(px + 2, py + 2, 4, 4, 1);
            } else if (t == G) {
                // Draw ghost door
                fillRect(px, py + 3, TILE_SIZE, 2, 1);
            }
        }
    }

    // Draw Score (starts at Y=120, 8 pixels tall)
    drawText(0, 120, "S:");
    drawText(12, 120, score);
    drawText(40, 120, "L:");
    drawText(52, 120, lives);

    // Draw Player
    const uint8_t* p_bmp = bmp_pacman_closed;
    // Animation toggle every 200ms
    if ((millis() / 200) % 2 == 0) {
        if (player.dir == DIR_LEFT) p_bmp = bmp_pacman_left;
        else if (player.dir == DIR_RIGHT) p_bmp = bmp_pacman_right;
        else if (player.dir == DIR_UP) p_bmp = bmp_pacman_up;
        else if (player.dir == DIR_DOWN) p_bmp = bmp_pacman_down;
    }
    drawSprite(offX + (int)player.x, offY + (int)player.y, p_bmp, 8, 8);

    // Draw Ghosts
    for (int i=0; i<MAX_GHOSTS; i++) {
        const uint8_t* g_bmp = ghosts[i].scared ? bmp_ghost_scared : bmp_ghost;
        // Bobbing animation
        if ((millis() / 200) % 2 == 0) g_bmp = ghosts[i].scared ? bmp_ghost_scared : bmp_ghost;
        drawSprite(offX + (int)ghosts[i].e.x, offY + (int)ghosts[i].e.y, g_bmp, 8, 8);
    }

    display_render();
}

void loop() {
    unsigned long now = millis();
    float dt = (now - lastFrameTime) / 1000.0f;
    lastFrameTime = now;
    if (dt > 0.1f) dt = 0.1f;

    bool fire_pressed = input_fire(); // Process 3s exit

    if (state == STATE_INTRO) {
        display_clear();
        drawText(10, 30, "PAC-MAN");
        drawText(2, 60, "PRESS FIRE");
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
        drawText(5, 30, "GAME OVER");
        drawText(0, 50, "Score:");
        drawText(40, 50, score);
        display_render();
        
        if (fire_pressed) {
            score = 0;
            lives = 3;
            state = STATE_INTRO;
        }
    } else if (state == STATE_VICTORY) {
        display_clear();
        drawText(16, 30, "MAZE");
        drawText(8, 40, "CLEARED!");
        drawText(2, 60, "PRESS FIRE");
        display_render();
        
        if (fire_pressed) {
            level++;
            initLevel();
            state = STATE_PLAYING;
            lastFrameTime = millis();
        }
    }
}}
