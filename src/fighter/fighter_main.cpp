#include "fighter_main.h"
#include "fighter_display.h"
#include "fighter_input.h"

#define BUZZER_PIN 14

namespace Fighter {

const int SCREEN_W = 128;
const int SCREEN_H = 64;
const float FLOOR_Y = 54.0f;

enum GameState {
    STATE_INTRO,
    STATE_ROUND_START,
    STATE_PLAYING,
    STATE_KO,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;
unsigned long stateTimer = 0;

int p1Wins = 0;
int p2Wins = 0;
int roundNum = 1;

enum ActionState {
    ACT_IDLE,
    ACT_WALK,
    ACT_CROUCH,
    ACT_JUMP,
    ACT_BLOCK,
    ACT_PUNCH,
    ACT_SWEEP,
    ACT_JUMPKICK,
    ACT_FIREBALL,
    ACT_HIT,
    ACT_KO
};

struct Character {
    float x, y;
    float vx, vy;
    int hp;
    int maxHp;
    ActionState action;
    unsigned long actionTimer;
    int dir; // 1 = right, -1 = left
    bool isPlayer;
};

Character p1;
Character p2;

struct Projectile {
    float x, y;
    float vx;
    bool active;
    int owner; // 1 = p1, 2 = p2
};

Projectile p1Fireball;
Projectile p2Fireball;

void initRound() {
    p1.x = 20; p1.y = FLOOR_Y; p1.vx = 0; p1.vy = 0;
    p1.hp = 100; p1.maxHp = 100;
    p1.action = ACT_IDLE; p1.dir = 1; p1.isPlayer = true;
    
    p2.x = 108; p2.y = FLOOR_Y; p2.vx = 0; p2.vy = 0;
    p2.hp = 100; p2.maxHp = 100;
    p2.action = ACT_IDLE; p2.dir = -1; p2.isPlayer = false;
    
    p1Fireball.active = false;
    p2Fireball.active = false;
}

void resetGame() {
    p1Wins = 0;
    p2Wins = 0;
    roundNum = 1;
    initRound();
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

// Check hitboxes
bool checkHit(float atkX, float atkY, float atkW, float atkH, Character& target) {
    if (target.action == ACT_KO) return false;
    
    float tX = target.x - 4; // Body width
    float tW = 8;
    float tY = target.action == ACT_CROUCH || target.action == ACT_BLOCK ? target.y - 10 : target.y - 16;
    float tH = target.action == ACT_CROUCH || target.action == ACT_BLOCK ? 10 : 16;
    
    if (atkX < tX + tW && atkX + atkW > tX && atkY < tY + tH && atkY + atkH > tY) {
        return true;
    }
    return false;
}

void applyHit(Character& target, int dmg, bool isHigh) {
    if (target.action == ACT_BLOCK && isHigh) {
        target.hp -= dmg / 4; // Chip damage
        tone(BUZZER_PIN, 400, 50);
        target.vx = target.dir * -10.0f; // Pushback
    } else {
        target.hp -= dmg;
        target.action = ACT_HIT;
        target.actionTimer = millis();
        target.vx = target.dir * -20.0f;
        tone(BUZZER_PIN, 1000, 100);
        if (target.hp <= 0) {
            target.hp = 0;
            target.action = ACT_KO;
            target.vx = target.dir * -40.0f;
            target.vy = -30.0f;
        }
    }
}

unsigned long lastTime = 0;
unsigned long cpuDecisionTimer = 0;
extern bool btn_held;
extern uint32_t btn_hold_start;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    bool btnRelease = input_button();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(25, 20, "MICRO FIGHTER");
        drawText(20, 40, "PRESS TO FIGHT!");
        display_render();
        
        if (btnRelease) {
            resetGame();
            state = STATE_ROUND_START;
            stateTimer = now;
            tone(BUZZER_PIN, 1500, 200);
        }
    } else if (state == STATE_ROUND_START) {
        display_clear();
        drawText(40, 25, "ROUND");
        drawText(75, 25, roundNum);
        drawText(45, 40, "FIGHT!");
        display_render();
        
        if (now - stateTimer > 2000) {
            state = STATE_PLAYING;
        }
    } else if (state == STATE_PLAYING) {
        
        // --- Player Input Logic ---
        if (p1.action != ACT_HIT && p1.action != ACT_KO && p1.action != ACT_FIREBALL) {
            bool onGround = (p1.y >= FLOOR_Y);
            
            if (onGround && p1.action != ACT_PUNCH && p1.action != ACT_SWEEP) {
                if (input_up()) {
                    p1.action = ACT_JUMP;
                    p1.vy = -70.0f;
                    tone(BUZZER_PIN, 800, 30);
                } else if (input_down()) {
                    // Check block
                    if (p1.dir == 1 && input_left()) p1.action = ACT_BLOCK;
                    else if (p1.dir == -1 && input_right()) p1.action = ACT_BLOCK;
                    else p1.action = ACT_CROUCH;
                    p1.vx = 0;
                } else if (input_left()) {
                    p1.action = ACT_WALK;
                    p1.vx = -40.0f;
                    if (p1.dir == -1) p1.action = ACT_BLOCK; // Walking away is block
                } else if (input_right()) {
                    p1.action = ACT_WALK;
                    p1.vx = 40.0f;
                    if (p1.dir == 1) p1.action = ACT_BLOCK; // Walking away is block
                } else {
                    p1.action = ACT_IDLE;
                    p1.vx = 0;
                }
            }
            
            // Attacks
            if (btnRelease) {
                unsigned long held = now - btn_hold_start;
                if (held >= 800 && onGround && !p1Fireball.active) {
                    p1.action = ACT_FIREBALL;
                    p1.actionTimer = now;
                    p1.vx = 0;
                    tone(BUZZER_PIN, 1500, 100);
                    
                    p1Fireball.active = true;
                    p1Fireball.x = p1.x + p1.dir * 10;
                    p1Fireball.y = p1.y - 12;
                    p1Fireball.vx = p1.dir * 80.0f;
                    p1Fireball.owner = 1;
                } else {
                    // Normal attacks
                    if (!onGround) {
                        if (p1.action != ACT_JUMPKICK) {
                            p1.action = ACT_JUMPKICK;
                            p1.actionTimer = now;
                            tone(BUZZER_PIN, 2000, 40);
                        }
                    } else if (input_down()) {
                        p1.action = ACT_SWEEP;
                        p1.actionTimer = now;
                        p1.vx = 0;
                        tone(BUZZER_PIN, 2000, 40);
                    } else {
                        p1.action = ACT_PUNCH;
                        p1.actionTimer = now;
                        p1.vx = 0;
                        tone(BUZZER_PIN, 2000, 40);
                    }
                }
            }
        }
        
        // --- CPU AI Logic ---
        if (p2.action != ACT_HIT && p2.action != ACT_KO && p2.action != ACT_FIREBALL && p2.y >= FLOOR_Y) {
            float dist = abs(p1.x - p2.x);
            
            if (now - cpuDecisionTimer > 200) { // Make decision every 200ms
                cpuDecisionTimer = now;
                int r = random(0, 100);
                
                if (dist > 60) {
                    if (r < 30 && !p2Fireball.active) {
                        p2.action = ACT_FIREBALL;
                        p2.actionTimer = now;
                        p2.vx = 0;
                        tone(BUZZER_PIN, 1500, 100);
                        
                        p2Fireball.active = true;
                        p2Fireball.x = p2.x + p2.dir * 10;
                        p2Fireball.y = p2.y - 12;
                        p2Fireball.vx = p2.dir * 80.0f;
                        p2Fireball.owner = 2;
                    } else if (r < 80) {
                        p2.action = ACT_WALK;
                        p2.vx = p2.dir * 30.0f; // Walk forward
                    } else {
                        p2.action = ACT_IDLE;
                        p2.vx = 0;
                    }
                } else if (dist < 25) {
                    if (r < 40) {
                        p2.action = ACT_PUNCH;
                        p2.actionTimer = now;
                        p2.vx = 0;
                        tone(BUZZER_PIN, 2000, 40);
                    } else if (r < 60) {
                        p2.action = ACT_SWEEP;
                        p2.actionTimer = now;
                        p2.vx = 0;
                        tone(BUZZER_PIN, 2000, 40);
                    } else if (r < 80) {
                        p2.action = ACT_BLOCK;
                        p2.vx = 0;
                    } else {
                        p2.action = ACT_WALK;
                        p2.vx = p2.dir * -30.0f; // Walk back
                    }
                } else {
                    if (r < 20) {
                        p2.action = ACT_JUMP;
                        p2.vy = -70.0f;
                        p2.vx = p2.dir * 40.0f; // Jump forward
                    } else if (r < 60) {
                        p2.action = ACT_WALK;
                        p2.vx = p2.dir * 30.0f;
                    } else {
                        p2.action = ACT_BLOCK;
                        p2.vx = 0;
                    }
                }
            }
        }
        
        // --- Physics & Action Updates ---
        Character* chars[2] = {&p1, &p2};
        for(int i=0; i<2; i++) {
            Character& c = *chars[i];
            Character& opp = *chars[1-i];
            
            // Facing direction
            if (c.y >= FLOOR_Y && c.action != ACT_HIT && c.action != ACT_KO && c.action != ACT_FIREBALL) {
                if (c.x < opp.x) c.dir = 1;
                else c.dir = -1;
            }
            
            // Gravity
            if (c.y < FLOOR_Y || c.action == ACT_KO) {
                c.vy += 150.0f * dt;
                c.y += c.vy * dt;
                c.x += c.vx * dt;
                
                if (c.y > FLOOR_Y && c.action != ACT_KO) {
                    c.y = FLOOR_Y;
                    c.vy = 0;
                    c.vx = 0;
                    if (c.action != ACT_HIT) c.action = ACT_IDLE;
                }
            } else {
                c.x += c.vx * dt;
            }
            
            // Wall bounds
            if (c.x < 5) c.x = 5;
            if (c.x > SCREEN_W - 5) c.x = SCREEN_W - 5;
            
            // Action Timers
            if (c.action == ACT_PUNCH || c.action == ACT_SWEEP) {
                if (now - c.actionTimer > 300) c.action = ACT_IDLE;
                else if (now - c.actionTimer > 100 && now - c.actionTimer < 150) {
                    // Active frames
                    float atkX = c.dir == 1 ? c.x + 4 : c.x - 12;
                    float atkY = c.action == ACT_PUNCH ? c.y - 14 : c.y - 4;
                    if (checkHit(atkX, atkY, 8, 4, opp)) {
                        applyHit(opp, 10, c.action == ACT_PUNCH);
                    }
                }
            }
            if (c.action == ACT_JUMPKICK) {
                // Active as long as falling
                if (c.vy > 0) {
                    float atkX = c.dir == 1 ? c.x + 4 : c.x - 10;
                    float atkY = c.y - 8;
                    if (checkHit(atkX, atkY, 6, 4, opp)) {
                        applyHit(opp, 12, true);
                        c.action = ACT_JUMP; // Prevents multi-hit
                    }
                }
            }
            if (c.action == ACT_FIREBALL) {
                if (now - c.actionTimer > 500) c.action = ACT_IDLE;
            }
            if (c.action == ACT_HIT) {
                if (now - c.actionTimer > 400) {
                    c.action = ACT_IDLE;
                    c.vx = 0;
                }
            }
        }
        
        // Character pushbox (can't walk through each other)
        if (p1.y >= FLOOR_Y && p2.y >= FLOOR_Y) {
            if (abs(p1.x - p2.x) < 10) {
                if (p1.x < p2.x) { p1.x -= 1; p2.x += 1; }
                else { p1.x += 1; p2.x -= 1; }
            }
        }
        
        // --- Projectile Update ---
        Projectile* projs[2] = {&p1Fireball, &p2Fireball};
        for(int i=0; i<2; i++) {
            if (projs[i]->active) {
                projs[i]->x += projs[i]->vx * dt;
                
                if (projs[i]->x < -10 || projs[i]->x > SCREEN_W + 10) {
                    projs[i]->active = false;
                } else {
                    Character& opp = (projs[i]->owner == 1) ? p2 : p1;
                    if (checkHit(projs[i]->x - 3, projs[i]->y - 3, 6, 6, opp)) {
                        applyHit(opp, 15, true); // High hit
                        projs[i]->active = false;
                    }
                }
            }
        }
        
        // --- Round End Logic ---
        if ((p1.action == ACT_KO || p2.action == ACT_KO) && state != STATE_KO) {
            state = STATE_KO;
            stateTimer = now;
            tone(BUZZER_PIN, 500, 1000);
        }
        
        // --- Draw ---
        display_clear();
        
        // Floor
        drawLine(0, FLOOR_Y, SCREEN_W, FLOOR_Y, 1);
        
        // UI
        // P1 Health
        drawRect(2, 2, 52, 6, 1);
        int p1w = (p1.hp * 50) / p1.maxHp;
        if (p1w > 0) fillRect(3, 3, p1w, 4, 1);
        // P2 Health
        drawRect(74, 2, 52, 6, 1);
        int p2w = (p2.hp * 50) / p2.maxHp;
        if (p2w > 0) fillRect(75 + (50 - p2w), 3, p2w, 4, 1);
        
        // Wins
        if (p1Wins >= 1) drawPixel(2, 10, 1);
        if (p1Wins >= 2) drawPixel(5, 10, 1);
        if (p2Wins >= 1) drawPixel(125, 10, 1);
        if (p2Wins >= 2) drawPixel(122, 10, 1);
        
        // Draw Characters
        for(int i=0; i<2; i++) {
            Character& c = *chars[i];
            int cx = (int)c.x;
            int cy = (int)c.y;
            
            // Torso
            if (c.action == ACT_CROUCH || c.action == ACT_BLOCK) {
                fillRect(cx-3, cy-10, 6, 10, 1);
            } else if (c.action == ACT_KO) {
                fillRect(cx-6, cy-4, 12, 4, 1);
            } else {
                fillRect(cx-3, cy-16, 6, 16, 1);
            }
            
            // Head
            if (c.action == ACT_CROUCH || c.action == ACT_BLOCK) {
                drawCircle(cx, cy-12, 2, 1);
            } else if (c.action == ACT_KO) {
                drawCircle(cx + (c.dir * -8), cy-2, 2, 1);
            } else {
                drawCircle(cx, cy-18, 2, 1);
            }
            
            // Arms
            if (c.action == ACT_IDLE || c.action == ACT_WALK || c.action == ACT_JUMP) {
                drawLine(cx, cy-12, cx + c.dir*4, cy-8, 1); // Front arm
            } else if (c.action == ACT_BLOCK) {
                drawLine(cx, cy-10, cx + c.dir*4, cy-10, 1); // Arm up
                drawLine(cx + c.dir*4, cy-10, cx + c.dir*4, cy-14, 1);
            } else if (c.action == ACT_PUNCH) {
                drawLine(cx, cy-12, cx + c.dir*8, cy-12, 1); // Punch straight
            } else if (c.action == ACT_FIREBALL) {
                drawLine(cx, cy-12, cx + c.dir*6, cy-12, 1); // Hands forward
            } else if (c.action == ACT_HIT) {
                drawLine(cx, cy-12, cx + c.dir*-4, cy-8, 1); // Arms back
            }
            
            // Legs
            if (c.action == ACT_WALK) {
                if ((now / 150) % 2 == 0) {
                    drawLine(cx, cy-4, cx-4, cy, 1);
                    drawLine(cx, cy-4, cx+4, cy, 1);
                } else {
                    drawLine(cx, cy-4, cx, cy, 1);
                }
            } else if (c.action == ACT_SWEEP) {
                drawLine(cx, cy-4, cx + c.dir*10, cy, 1); // Leg out
            } else if (c.action == ACT_JUMPKICK) {
                drawLine(cx, cy-4, cx + c.dir*8, cy-2, 1); // Kick in air
            } else if (c.action == ACT_KO) {
                drawLine(cx+4, cy-2, cx+8, cy-2, 1); // Legs flat
            }
        }
        
        // Draw Projectiles
        for(int i=0; i<2; i++) {
            if (projs[i]->active) {
                drawCircle((int)projs[i]->x, (int)projs[i]->y, 3, 1);
                if ((now / 50) % 2 == 0) fillCircle((int)projs[i]->x, (int)projs[i]->y, 2, 1); // Flash
            }
        }
        
        display_render();
        
    } else if (state == STATE_KO) {
        display_clear();
        drawText(55, 30, "KO!");
        display_render();
        
        if (now - stateTimer > 3000) {
            if (p1.hp > 0) p1Wins++;
            else p2Wins++;
            
            if (p1Wins >= 2 || p2Wins >= 2) {
                stateTimer = now;
                state = STATE_GAMEOVER;
            } else {
                roundNum++;
                initRound();
                stateTimer = now;
                state = STATE_ROUND_START;
            }
        }
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        if (p1Wins >= 2) drawText(35, 20, "YOU WIN!");
        else drawText(35, 20, "YOU LOSE");
        drawText(15, 40, "PRESS TO RESTART");
        display_render();
        
        if (btnRelease && now - stateTimer > 1000) {
            state = STATE_INTRO;
        }
    }
}

}
