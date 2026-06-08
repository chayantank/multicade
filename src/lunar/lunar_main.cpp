#include "lunar_main.h"
#include "lunar_display.h"
#include "lunar_input.h"
#include <Arduino.h>
#include <math.h>

#define BUZZER_PIN 14

namespace Lunar {

const int SCREEN_W = 128;
const int SCREEN_H = 64;

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_CRASH,
    STATE_LANDED,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;
unsigned long stateTimer = 0;

int score = 0;
int level = 1;

// Ship
float pX = 64.0f;
float pY = 10.0f;
float vX = 0.0f;
float vY = 0.0f;
float angle = 0.0f; // Radians
float fuel = 100.0f;

// Terrain
const int NUM_POINTS = 17; // 128 / 8 = 16 segments
int tY[NUM_POINTS];
int padIndex = 0;

void generateTerrain() {
    padIndex = random(2, NUM_POINTS - 3); // Choose a random flat segment
    int padHeight = random(40, 60);
    
    tY[0] = random(30, 60);
    for(int i=1; i<NUM_POINTS; i++) {
        if (i == padIndex || i == padIndex + 1 || i == padIndex + 2) {
            tY[i] = padHeight; // Flat pad width = 2 segments
        } else {
            tY[i] = tY[i-1] + random(-15, 15);
            if (tY[i] < 20) tY[i] = 20;
            if (tY[i] > 63) tY[i] = 63;
        }
    }
}

void initLevel() {
    pX = 10.0f;
    pY = 10.0f;
    vX = 20.0f; // Initial push
    vY = 0.0f;
    angle = 0.0f;
    fuel = 100.0f - (level * 5.0f);
    if (fuel < 30.0f) fuel = 30.0f;
    generateTerrain();
}

void resetGame() {
    score = 0;
    level = 1;
    initLevel();
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

unsigned long lastTime = 0;

bool lineIntersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    float uA = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
    float uB = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
    return (uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1);
}

void rotatePoint(float cx, float cy, float &x, float &y, float a) {
    float s = sin(a);
    float c = cos(a);
    x -= cx;
    y -= cy;
    float nx = x * c - y * s;
    float ny = x * s + y * c;
    x = nx + cx;
    y = ny + cy;
}

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    bool thrust = input_thrust();
    bool left = input_left();
    bool right = input_right();

    if (state == STATE_INTRO) {
        display_clear();
        drawText(30, 20, "LUNAR LANDER");
        drawText(20, 40, "PRESS TO START");
        display_render();
        
        if (thrust) {
            resetGame();
            state = STATE_PLAYING;
            tone(BUZZER_PIN, 1000, 100);
        }
    } else if (state == STATE_PLAYING) {
        
        // Input
        if (left) angle -= 3.0f * dt;
        if (right) angle += 3.0f * dt;
        
        // Thrust
        if (thrust && fuel > 0) {
            fuel -= 15.0f * dt;
            vX += sin(angle) * 40.0f * dt;
            vY -= cos(angle) * 40.0f * dt;
            if (now % 100 < 50) tone(BUZZER_PIN, 100, 20); // Engine sound
        }
        
        // Gravity
        vY += 15.0f * dt;
        
        // Movement
        pX += vX * dt;
        pY += vY * dt;
        
        // Screen bounds
        if (pX < 0) { pX = 0; vX = 0; }
        if (pX > SCREEN_W) { pX = SCREEN_W; vX = 0; }
        if (pY < 0) { pY = 0; vY = 0; }
        
        // Ship geometry (triangle pointing up)
        float s1x = pX, s1y = pY - 4;
        float s2x = pX - 3, s2y = pY + 3;
        float s3x = pX + 3, s3y = pY + 3;
        
        rotatePoint(pX, pY, s1x, s1y, angle);
        rotatePoint(pX, pY, s2x, s2y, angle);
        rotatePoint(pX, pY, s3x, s3y, angle);
        
        // Collision
        bool crashed = false;
        bool landed = false;
        
        for(int i=0; i<NUM_POINTS-1; i++) {
            float tx1 = i * 8;
            float ty1 = tY[i];
            float tx2 = (i+1) * 8;
            float ty2 = tY[i+1];
            
            // Check intersection of ship bottom with terrain segment
            if (lineIntersect(s2x, s2y, s3x, s3y, tx1, ty1, tx2, ty2)) {
                // Determine if landing pad
                if ((i == padIndex || i == padIndex + 1) && abs(angle) < 0.2f && abs(vY) < 15.0f && abs(vX) < 10.0f) {
                    landed = true;
                } else {
                    crashed = true;
                }
            }
            // Check other sides just in case
            if (!landed && !crashed) {
                if (lineIntersect(s1x, s1y, s2x, s2y, tx1, ty1, tx2, ty2) ||
                    lineIntersect(s1x, s1y, s3x, s3y, tx1, ty1, tx2, ty2)) {
                    crashed = true;
                }
            }
        }
        
        if (crashed) {
            state = STATE_CRASH;
            stateTimer = now;
            tone(BUZZER_PIN, 100, 1000);
        } else if (landed) {
            score += (int)fuel * 10 + 100;
            state = STATE_LANDED;
            stateTimer = now;
            tone(BUZZER_PIN, 1500, 500);
        }
        
        // Draw
        display_clear();
        
        // Terrain
        for(int i=0; i<NUM_POINTS-1; i++) {
            drawLine(i*8, tY[i], (i+1)*8, tY[i+1], 1);
        }
        // Highlight pad
        drawLine(padIndex*8, tY[padIndex]+1, (padIndex+2)*8, tY[padIndex+2]+1, 1);
        
        // Ship
        drawTriangle((int)s1x, (int)s1y, (int)s2x, (int)s2y, (int)s3x, (int)s3y, 1);
        
        // Thrust flame
        if (thrust && fuel > 0) {
            float fx = pX, fy = pY + 8;
            rotatePoint(pX, pY, fx, fy, angle);
            drawLine((int)s2x, (int)s2y, (int)fx, (int)fy, 1);
            drawLine((int)s3x, (int)s3y, (int)fx, (int)fy, 1);
        }
        
        // HUD
        fillRect(0, 0, SCREEN_W, 9, 0);
        drawText(2, 1, "S:"); drawText(15, 1, score);
        drawText(60, 1, "F:"); 
        drawRect(75, 1, 50, 6, 1);
        if (fuel > 0) fillRect(76, 2, (int)((fuel/100.0f)*48.0f), 4, 1);
        
        display_render();
        
    } else if (state == STATE_CRASH) {
        display_clear();
        drawText(40, 20, "CRASHED!");
        drawText(20, 40, "PRESS TO RESTART");
        display_render();
        if (thrust && now - stateTimer > 1000) {
            state = STATE_INTRO;
        }
    } else if (state == STATE_LANDED) {
        display_clear();
        drawText(25, 20, "SAFE LANDING!");
        drawText(30, 40, "SCORE:"); drawText(70, 40, score);
        display_render();
        if (thrust && now - stateTimer > 1000) {
            level++;
            initLevel();
            state = STATE_PLAYING;
        }
    }
}

}
