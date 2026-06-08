#include "punchout_main.h"
#include "punchout_display.h"
#include "punchout_input.h"

#define BUZZER_PIN 14

namespace PunchOut {

const int SCREEN_W = 64;
const int SCREEN_H = 128;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_WIN,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;
unsigned long stateTimer = 0;

enum OpponentState {
    OPP_IDLE,
    OPP_TELEGRAPH_L,
    OPP_TELEGRAPH_R,
    OPP_ATTACK_L,
    OPP_ATTACK_R,
    OPP_VULNERABLE,
    OPP_STUNNED
};

OpponentState oppState = OPP_IDLE;
unsigned long oppTimer = 0;
int oppHp = 10;

int playerDodge = 0; // -1=L, 0=Center, 1=R
bool playerPunching = false;
unsigned long playerPunchTimer = 0;
int playerHp = 3;

void setOpponentState(OpponentState newState) {
    oppState = newState;
    oppTimer = millis();
}

void resetGame() {
    playerHp = 3;
    oppHp = 10;
    setOpponentState(OPP_IDLE);
    playerDodge = 0;
    playerPunching = false;
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

void drawOpponent() {
    int cx = 32;
    int cy = 60; // Center of opponent body
    
    // Body
    drawRect(cx - 15, cy - 20, 30, 40, 1);
    // Head
    drawCircle(cx, cy - 30, 8, 1);
    
    // Left Arm (Opponent's Right)
    if (oppState == OPP_TELEGRAPH_R) {
        drawCircle(cx - 20, cy - 30, 6, 1); // Pulled back high
    } else if (oppState == OPP_ATTACK_R) {
        drawCircle(cx - 10, cy + 20, 8, 1); // Punched forward (down towards player)
        fillCircle(cx - 10, cy + 20, 6, 1);
    } else {
        drawCircle(cx - 20, cy, 6, 1); // Idle
    }
    
    // Right Arm (Opponent's Left)
    if (oppState == OPP_TELEGRAPH_L) {
        drawCircle(cx + 20, cy - 30, 6, 1); // Pulled back high
    } else if (oppState == OPP_ATTACK_L) {
        drawCircle(cx + 10, cy + 20, 8, 1); // Punched forward
        fillCircle(cx + 10, cy + 20, 6, 1);
    } else {
        drawCircle(cx + 20, cy, 6, 1); // Idle
    }
    
    // Stunned stars
    if (oppState == OPP_STUNNED || oppState == OPP_VULNERABLE) {
        int sx = cx + (millis() % 20) - 10;
        int sy = cy - 40;
        drawLine(sx-2, sy, sx+2, sy, 1);
        drawLine(sx, sy-2, sx, sy+2, 1);
    }
}

void drawPlayer() {
    int px = 32;
    int py = 110;
    
    if (playerDodge == -1) px = 15;
    else if (playerDodge == 1) px = 49;
    
    // Head
    drawCircle(px, py, 6, 1);
    
    // Glove
    if (playerPunching) {
        // Punched forward
        fillCircle(32, 80, 5, 1);
        drawLine(px, py-6, 32, 85, 1);
    } else {
        // Gloves up
        drawCircle(px - 6, py - 4, 4, 1);
        drawCircle(px + 6, py - 4, 4, 1);
    }
}

unsigned long lastTime = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    bool fire = input_punch();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(5, 40, "MICRO PUNCH");
        drawText(5, 70, "PRESS TO");
        drawText(15, 80, "START");
        display_render();
        
        if (fire) {
            resetGame();
            state = STATE_PLAYING;
        }
    } else if (state == STATE_PLAYING) {
        
        // --- Player Input ---
        playerDodge = input_dodge();
        
        if (fire && !playerPunching && playerDodge == 0) {
            playerPunching = true;
            playerPunchTimer = now;
            tone(BUZZER_PIN, 800, 50);
            
            // Check hit
            if (oppState == OPP_VULNERABLE || oppState == OPP_STUNNED) {
                oppHp--;
                setOpponentState(OPP_STUNNED);
                tone(BUZZER_PIN, 1200, 100);
                if (oppHp <= 0) {
                    state = STATE_WIN;
                    stateTimer = now;
                }
            } else {
                // Blocked
                tone(BUZZER_PIN, 200, 50);
            }
        }
        
        if (playerPunching && now - playerPunchTimer > 200) {
            playerPunching = false;
        }
        
        // --- Opponent AI ---
        unsigned long elapsed = now - oppTimer;
        
        if (oppState == OPP_IDLE) {
            if (elapsed > 1000) {
                if (random(0, 100) < 50) setOpponentState(OPP_TELEGRAPH_L);
                else setOpponentState(OPP_TELEGRAPH_R);
            }
        } else if (oppState == OPP_TELEGRAPH_L) {
            if (elapsed > 600) { // Telegraph time
                setOpponentState(OPP_ATTACK_L);
                tone(BUZZER_PIN, 150, 100);
                // Check if player is dodging right
                if (playerDodge != 1) {
                    playerHp--;
                    tone(BUZZER_PIN, 100, 500);
                    if (playerHp <= 0) { state = STATE_GAMEOVER; stateTimer = now; }
                }
            }
        } else if (oppState == OPP_TELEGRAPH_R) {
            if (elapsed > 600) {
                setOpponentState(OPP_ATTACK_R);
                tone(BUZZER_PIN, 150, 100);
                // Check if player is dodging left
                if (playerDodge != -1) {
                    playerHp--;
                    tone(BUZZER_PIN, 100, 500);
                    if (playerHp <= 0) { state = STATE_GAMEOVER; stateTimer = now; }
                }
            }
        } else if (oppState == OPP_ATTACK_L || oppState == OPP_ATTACK_R) {
            if (elapsed > 300) {
                setOpponentState(OPP_VULNERABLE);
            }
        } else if (oppState == OPP_VULNERABLE) {
            if (elapsed > 800) {
                setOpponentState(OPP_IDLE);
            }
        } else if (oppState == OPP_STUNNED) {
            if (elapsed > 600) {
                setOpponentState(OPP_IDLE);
            }
        }
        
        // --- Render ---
        display_clear();
        
        drawOpponent();
        drawPlayer();
        
        // HUD
        drawText(2, 2, "HP:");
        for(int i=0; i<playerHp; i++) fillRect(20 + i*6, 2, 4, 4, 1);
        
        drawText(35, 2, oppHp);
        
        display_render();
        
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(5, 40, "KNOCKOUT");
        display_render();
        if (fire && now - stateTimer > 1000) state = STATE_INTRO;
    } else if (state == STATE_WIN) {
        display_clear();
        drawText(5, 40, "YOU WIN!");
        display_render();
        if (fire && now - stateTimer > 1000) state = STATE_INTRO;
    }
}

}
