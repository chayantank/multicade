#include "rps_main.h"
#include "rps_display.h"
#include "rps_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace RPS {

const int SCREEN_W = 128;
const int SCREEN_H = 64;

enum GameState {
    STATE_SELECT,
    STATE_THROW_ANIM,
    STATE_REVEAL,
    STATE_RESULT,
    STATE_GAMEOVER
};

GameState state = STATE_SELECT;

const char* choices[] = {"ROCK", "PAPER", "SCISSOR"};
int playerChoice = 0;
int cpuChoice = 0;
int resultStatus = 0; // 0=Draw, 1=Win, -1=Loss

int playerHealth = 100;
int cpuHealth = 100;

unsigned long stateTimer = 0;
unsigned long lastMoveTime = 0;
bool lastFire = false;

int throwFrame = 0;
int shakeAmount = 0;

void setup() {
    display_setup();
    input_setup();
    state = STATE_SELECT;
    playerHealth = 100;
    cpuHealth = 100;
}

void loop() {
    bool currentFire = input_fire();

    if (state == STATE_SELECT) {
        if (millis() - lastMoveTime > 200) {
            if (input_up()) {
                playerChoice--;
                if (playerChoice < 0) playerChoice = 2;
                lastMoveTime = millis();
                tone(BUZZER_PIN, 400, 20);
            } else if (input_down()) {
                playerChoice++;
                if (playerChoice > 2) playerChoice = 0;
                lastMoveTime = millis();
                tone(BUZZER_PIN, 400, 20);
            }
        }

        display_clear();
        drawText(30, 2, "BATTLE RPS");
        
        for (int i = 0; i < 3; i++) {
            int y = 20 + (i * 14);
            if (i == playerChoice) {
                fillRect(5, y - 2, 60, 12, 1);
                drawTextInverted(15, y, choices[i]);
            } else {
                drawText(15, y, choices[i]);
            }
        }

        drawText(75, 20, "YOU");
        drawRect(75, 30, 40, 6, 1);
        fillRect(76, 31, (playerHealth * 38) / 100, 4, 1);
        
        drawText(75, 40, "CPU");
        drawRect(75, 50, 40, 6, 1);
        fillRect(76, 51, (cpuHealth * 38) / 100, 4, 1);

        display_render();

        if (currentFire && !lastFire) {
            cpuChoice = random(0, 3);
            state = STATE_THROW_ANIM;
            stateTimer = millis();
            throwFrame = 0;
            tone(BUZZER_PIN, 800, 100);
        }
        
    } else if (state == STATE_THROW_ANIM) {
        display_clear();
        unsigned long elapsed = millis() - stateTimer;
        
        if (elapsed < 600) { drawTextLarge(50, 25, "3"); if(throwFrame==0) { throwFrame++; tone(BUZZER_PIN, 800, 50); } }
        else if (elapsed < 1200) { drawTextLarge(50, 25, "2"); if(throwFrame==1) { throwFrame++; tone(BUZZER_PIN, 800, 50); } }
        else if (elapsed < 1800) { drawTextLarge(50, 25, "1"); if(throwFrame==2) { throwFrame++; tone(BUZZER_PIN, 800, 50); } }
        else {
            state = STATE_REVEAL;
            stateTimer = millis();
            tone(BUZZER_PIN, 1500, 200);
        }
        display_render();

    } else if (state == STATE_REVEAL) {
        display_clear();
        drawTextLarge(35, 25, "SHOOT!");
        display_render();

        if (millis() - stateTimer > 800) {
            if (playerChoice == cpuChoice) {
                resultStatus = 0;
            } else if ((playerChoice == 0 && cpuChoice == 2) || 
                       (playerChoice == 1 && cpuChoice == 0) || 
                       (playerChoice == 2 && cpuChoice == 1)) {
                resultStatus = 1;
                cpuHealth -= 34; 
                tone(BUZZER_PIN, 1200, 200);
            } else {
                resultStatus = -1;
                playerHealth -= 34;
                tone(BUZZER_PIN, 100, 400);
            }
            shakeAmount = 10;
            state = STATE_RESULT;
            stateTimer = millis();
        }
        
    } else if (state == STATE_RESULT) {
        display_clear();
        
        int ox = 0, oy = 0;
        if (resultStatus != 0 && shakeAmount > 0) {
            ox = random(-shakeAmount, shakeAmount);
            oy = random(-shakeAmount, shakeAmount);
            shakeAmount--;
        }
        
        drawText(5 + ox, 10 + oy, "YOU:");
        drawText(35 + ox, 10 + oy, choices[playerChoice]);
        
        drawText(5 + ox, 25 + oy, "CPU:");
        drawText(35 + ox, 25 + oy, choices[cpuChoice]);
        
        if (resultStatus == 1) {
            drawTextLarge(15 + ox, 45 + oy, "YOU WIN!");
        } else if (resultStatus == -1) {
            drawTextLarge(10 + ox, 45 + oy, "YOU LOSE!");
        } else {
            drawTextLarge(35 + ox, 45 + oy, "DRAW!");
        }

        display_render();

        if (millis() - stateTimer > 1000 && currentFire && !lastFire) {
            if (playerHealth <= 0 || cpuHealth <= 0) {
                state = STATE_GAMEOVER;
                stateTimer = millis();
            } else {
                state = STATE_SELECT;
            }
        }
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        if (playerHealth <= 0) {
            drawTextLarge(15, 25, "DEFEAT!");
        } else {
            drawTextLarge(15, 25, "VICTORY!");
        }
        drawText(15, 50, "PRESS TO RESTART");
        display_render();
        
        if (millis() - stateTimer > 1000 && currentFire && !lastFire) {
            setup();
        }
    }
    
    lastFire = currentFire;
}

}
