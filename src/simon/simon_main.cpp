#include "simon_main.h"
#include "simon_display.h"
#include "simon_input.h"

#define BUZZER_PIN 14

namespace Simon {

const int SCREEN_W = 128;
const int SCREEN_H = 64;

enum GameState {
    STATE_INTRO,
    STATE_WATCH_SEQUENCE,
    STATE_PLAYER_TURN,
    STATE_GAMEOVER,
    STATE_WIN
};

GameState state = STATE_INTRO;

const int MAX_SEQUENCE = 50;
int sequence[MAX_SEQUENCE];
int currentLength = 1;
int playerIndex = 0;

int watchIndex = 0;
unsigned long stateTimer = 0;
bool isBlinking = false;

// 0=Up, 1=Right, 2=Down, 3=Left
const int TONES[] = {440, 554, 659, 880}; 

void addNextNote() {
    if (currentLength < MAX_SEQUENCE) {
        sequence[currentLength - 1] = random(0, 4);
    }
}

void resetGame() {
    currentLength = 1;
    addNextNote();
    state = STATE_WATCH_SEQUENCE;
    watchIndex = 0;
    isBlinking = false;
    stateTimer = millis();
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

void drawQuadrants(int activeQuadrant) {
    // Up: top middle
    int qx[4] = {48, 80, 48, 16};
    int qy[4] = {0, 16, 32, 16};
    
    for (int i = 0; i < 4; i++) {
        if (activeQuadrant == i) {
            fillRect(qx[i], qy[i], 32, 32, 1);
        } else {
            drawRect(qx[i], qy[i], 32, 32, 1);
            // Inner hollow box
            drawRect(qx[i] + 4, qy[i] + 4, 24, 24, 1);
        }
    }
}

void loop() {
    unsigned long now = millis();
    bool fire = input_fire();
    int dir = input_get_direction();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(35, 10, "SIMON SAYS");
        drawText(20, 30, "PRESS TO START");
        display_render();
        
        if (fire) {
            resetGame();
        }
    } else if (state == STATE_WATCH_SEQUENCE) {
        display_clear();
        drawText(5, 5, "WATCH");
        drawText(100, 5, currentLength);
        
        if (watchIndex < currentLength) {
            if (!isBlinking) {
                if (now - stateTimer > 250) { // Gap between notes
                    isBlinking = true;
                    stateTimer = now;
                    tone(BUZZER_PIN, TONES[sequence[watchIndex]], 400);
                }
            } else {
                if (now - stateTimer < 400) {
                    drawQuadrants(sequence[watchIndex]);
                } else {
                    isBlinking = false;
                    watchIndex++;
                    stateTimer = now;
                }
            }
        } else {
            if (now - stateTimer > 500) {
                state = STATE_PLAYER_TURN;
                playerIndex = 0;
            }
            drawQuadrants(-1);
        }
        
        display_render();
        
    } else if (state == STATE_PLAYER_TURN) {
        display_clear();
        drawText(5, 5, "YOUR TURN");
        drawText(100, 5, currentLength);
        
        if (dir != -1) {
            tone(BUZZER_PIN, TONES[dir], 200);
            
            if (dir == sequence[playerIndex]) {
                // Correct
                playerIndex++;
                if (playerIndex == currentLength) {
                    if (currentLength >= MAX_SEQUENCE) {
                        state = STATE_WIN;
                        tone(BUZZER_PIN, 2000, 500);
                    } else {
                        currentLength++;
                        addNextNote();
                        state = STATE_WATCH_SEQUENCE;
                        watchIndex = 0;
                        isBlinking = false;
                        stateTimer = now + 500; // Small delay before next sequence
                    }
                }
            } else {
                // Wrong
                state = STATE_GAMEOVER;
                tone(BUZZER_PIN, 100, 1000);
            }
        }
        
        drawQuadrants(dir); // Highlight briefly if pressing
        display_render();
        
    } else if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(35, 20, "GAME OVER");
        drawText(20, 40, "SCORE:");
        drawText(60, 40, currentLength - 1);
        display_render();
        
        if (fire) {
            state = STATE_INTRO;
        }
    } else if (state == STATE_WIN) {
        display_clear();
        drawText(35, 20, "YOU WIN!");
        display_render();
        
        if (fire) {
            state = STATE_INTRO;
        }
    }
}

}
