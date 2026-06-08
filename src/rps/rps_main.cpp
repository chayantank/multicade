#include "rps_main.h"
#include "rps_display.h"
#include "rps_input.h"

#define BUZZER_PIN 14

namespace RPS {

const int SCREEN_W = 128;
const int SCREEN_H = 64;

enum GameState {
    STATE_SELECT,
    STATE_REVEAL,
    STATE_RESULT
};

GameState state = STATE_SELECT;

const char* choices[] = {"ROCK", "PAPER", "SCISSOR"};
int playerChoice = 0;
int cpuChoice = 0;
int resultStatus = 0; // 0=Draw, 1=Win, -1=Loss

int playerScore = 0;
int cpuScore = 0;

unsigned long stateTimer = 0;
unsigned long lastMoveTime = 0;
bool lastFire = false;

void setup() {
    display_setup();
    input_setup();
    state = STATE_SELECT;
    playerScore = 0;
    cpuScore = 0;
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
        drawText(35, 2, "MAKE CHOICE");
        
        for (int i = 0; i < 3; i++) {
            int y = 20 + (i * 14);
            if (i == playerChoice) {
                fillRect(10, y - 2, 70, 12, 1);
                drawTextInverted(15, y, choices[i]);
            } else {
                drawText(15, y, choices[i]);
            }
        }

        drawText(95, 25, "YOU:");
        drawText(120, 25, playerScore);
        drawText(95, 40, "CPU:");
        drawText(120, 40, cpuScore);

        display_render();

        if (currentFire && !lastFire) {
            cpuChoice = random(0, 3);
            state = STATE_REVEAL;
            stateTimer = millis();
            tone(BUZZER_PIN, 800, 100);
        }
        
    } else if (state == STATE_REVEAL) {
        display_clear();
        drawText(35, 25, "SHOOT!");
        display_render();

        if (millis() - stateTimer > 1000) {
            // Calculate winner
            if (playerChoice == cpuChoice) {
                resultStatus = 0;
            } else if ((playerChoice == 0 && cpuChoice == 2) || 
                       (playerChoice == 1 && cpuChoice == 0) || 
                       (playerChoice == 2 && cpuChoice == 1)) {
                resultStatus = 1;
                playerScore++;
                tone(BUZZER_PIN, 1200, 200);
            } else {
                resultStatus = -1;
                cpuScore++;
                tone(BUZZER_PIN, 100, 400);
            }
            state = STATE_RESULT;
        }
        
    } else if (state == STATE_RESULT) {
        display_clear();
        
        drawText(5, 10, "YOU:");
        drawText(35, 10, choices[playerChoice]);
        
        drawText(5, 25, "CPU:");
        drawText(35, 25, choices[cpuChoice]);
        
        if (resultStatus == 1) {
            drawTextLarge(15, 45, "YOU WIN!");
        } else if (resultStatus == -1) {
            drawTextLarge(10, 45, "YOU LOSE!");
        } else {
            drawTextLarge(35, 45, "DRAW!");
        }

        display_render();

        if (currentFire && !lastFire) {
            state = STATE_SELECT;
        }
    }
    
    lastFire = currentFire;
}

}
