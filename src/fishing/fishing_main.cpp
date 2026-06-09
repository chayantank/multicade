#include "fishing_main.h"
#include "fishing_display.h"
#include "fishing_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Fishing {

enum FishType { NORMAL, ERRATIC, HEAVY };

float barY = 50;
float barVy = 0;
float barHeight = 15;

float fishY = 50;
float fishVy = 0;
float fishTargetY = 50;
FishType currentType = NORMAL;

struct Chest {
    float y;
    float progress;
    bool active;
};
Chest chest = {0, 0, false};

float captureProgress = 0;
int catches = 0;
int score = 0;
bool gameOver = false;

unsigned long lastTime = 0;
int fishTimer = 0;

void resetGame() {
    barY = 50; barVy = 0;
    fishY = 50; fishVy = 0;
    captureProgress = 0;
    catches = 0; score = 0;
    gameOver = false;
    currentType = NORMAL;
    chest.active = false;
}

void setup() {
    display_setup();
    input_setup();
    resetGame();
}

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.05f) dt = 0.05f;
    
    if (gameOver) {
        display_clear();
        drawText(35, 20, "FISH ESCAPED!");
        drawText(30, 40, "SCORE:");
        drawText(70, 40, score);
        display_render();
        if (input_action() && now % 500 < 50) resetGame();
        return;
    }
    
    if (captureProgress >= 100.0f) {
        catches++;
        score += 100;
        tone(BUZZER_PIN, 1200, 100); delay(100); tone(BUZZER_PIN, 1500, 100);
        captureProgress = 0;
        barHeight = max(5.0f, 15.0f - catches * 0.5f);
        
        int r = random(0, 3);
        if (r==0) currentType = NORMAL;
        else if (r==1) currentType = ERRATIC;
        else currentType = HEAVY;
        
        if (random(0, 3) == 0) {
            chest.active = true;
            chest.y = random(10, 50);
            chest.progress = 0;
        }
    }
    
    if (input_action()) barVy -= 120.0f * dt;
    else barVy += 120.0f * dt;
    barVy *= 0.9f;
    barY += barVy * dt;
    
    if (barY < 0) { barY = 0; barVy = 0; }
    if (barY > 64 - barHeight) { barY = 64 - barHeight; barVy = 0; }
    
    fishTimer--;
    if (fishTimer <= 0) {
        if (currentType == ERRATIC) fishTimer = random(10, 40);
        else fishTimer = random(30, 100);
        fishTargetY = random(0, 60);
    }
    
    float diff = fishTargetY - fishY;
    if (currentType == HEAVY) {
        if (diff > 0) fishVy += (diff * 4.0f) * dt;
        else fishVy += (diff * 1.0f) * dt;
    } else if (currentType == ERRATIC) {
        fishVy += (diff * 5.0f) * dt;
    } else {
        fishVy += (diff * 2.0f) * dt;
    }
    
    fishVy *= (currentType == ERRATIC ? 0.8f : 0.95f);
    
    float difficultyMult = 1.0f + catches * 0.1f;
    fishY += fishVy * dt * difficultyMult;
    
    if (fishY < 2) { fishY = 2; fishVy = 0; }
    if (fishY > 62) { fishY = 62; fishVy = 0; }
    
    if (fishY > barY && fishY < barY + barHeight) {
        captureProgress += 15.0f * dt;
        tone(BUZZER_PIN, 2000, 10);
    } else {
        captureProgress -= 10.0f * dt;
    }
    
    if (chest.active) {
        if (chest.y > barY && chest.y < barY + barHeight) {
            chest.progress += 30.0f * dt;
            tone(BUZZER_PIN, 3000, 10);
            if (chest.progress >= 100.0f) {
                chest.active = false;
                score += 500;
                tone(BUZZER_PIN, 2500, 150);
            }
        } else {
            chest.progress -= 10.0f * dt;
            if (chest.progress < 0) chest.progress = 0;
        }
    }
    
    if (captureProgress < 0) {
        gameOver = true;
        tone(BUZZER_PIN, 100, 500);
    }
    
    display_clear();
    
    drawRect(40, 0, 10, 64, 1);
    fillRect(41, (int)barY, 8, (int)barHeight, 1);
    
    fillRect(43, (int)fishY - 1, 4, 3, 0);
    drawRect(42, (int)fishY - 2, 6, 5, 1);
    if (currentType == ERRATIC) drawPixel(44, (int)fishY, 1);
    if (currentType == HEAVY) fillRect(43, (int)fishY - 1, 4, 3, 1);
    
    if (chest.active) {
        drawRect(42, (int)chest.y, 6, 4, 1);
        drawLine(42, (int)chest.y+1, 47, (int)chest.y+1, 1);
        int cpHeight = (chest.progress / 100.0f) * 4;
        fillRect(36, (int)chest.y + 4 - cpHeight, 2, cpHeight, 1);
    }
    
    drawRect(60, 10, 8, 44, 1);
    float pHeight = (captureProgress / 100.0f) * 42.0f;
    fillRect(61, 53 - (int)pHeight, 6, (int)pHeight, 1);
    
    drawText(80, 20, "SCORE");
    drawText(80, 30, score);
    
    display_render();
}
}
