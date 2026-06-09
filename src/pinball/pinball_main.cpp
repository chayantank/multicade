#include "pinball_main.h"
#include "pinball_display.h"
#include "pinball_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Pinball {

enum GameState {
    STATE_INTRO,
    STATE_PLAYING,
    STATE_GAMEOVER
};

GameState state = STATE_INTRO;

struct Ball {
    float x, y;
    float vx, vy;
    float r;
    bool active;
};

Ball ball;

struct Bumper {
    float x, y, r;
};

const int NUM_BUMPERS = 3;
Bumper bumpers[NUM_BUMPERS] = {
    {32, 20, 6}, {64, 15, 6}, {96, 20, 6}
};

struct Target {
    float x, y, vx, r;
    bool active;
};
Target movingTarget = {64, 8, 40.0f, 4.0f, true};

float leftFlipAngle = 0.2f; 
float rightFlipAngle = 2.94f; 

float lfx = 40, lfy = 60;
float rfx = 88, rfy = 60;
float flipLen = 16.0f;

int score = 0;
int balls = 3;
bool plunging = true;

void resetGame() {
    score = 0;
    balls = 3;
    plunging = true;
    ball.x = 120;
    ball.y = 50;
    ball.vx = 0;
    ball.vy = 0;
    ball.r = 2.0f;
    ball.active = true;
    state = STATE_PLAYING;
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

void resolveCircleCollision(float& bx, float& by, float& bvx, float& bvy, float cx, float cy, float cr, int pts) {
    float dx = bx - cx;
    float dy = by - cy;
    float dist = sqrt(dx*dx + dy*dy);
    if (dist < cr + ball.r) {
        float nx = dx / dist;
        float ny = dy / dist;
        
        bx = cx + nx * (cr + ball.r);
        
        float dot = bvx * nx + bvy * ny;
        if (dot < 0) {
            bvx = bvx - 2 * dot * nx;
            bvy = bvy - 2 * dot * ny;
            bvx += nx * 100.0f;
            bvy += ny * 100.0f;
            score += pts;
            if (pts > 50) {
                tone(BUZZER_PIN, 2000, 100);
            } else {
                tone(BUZZER_PIN, 1200, 50);
            }
        }
    }
}

void resolveLineCollision(float& bx, float& by, float& bvx, float& bvy, float x1, float y1, float x2, float y2, bool isFlipper) {
    float lx = x2 - x1;
    float ly = y2 - y1;
    float len = sqrt(lx*lx + ly*ly);
    float nx = -ly / len;
    float ny = lx / len;
    
    float bdx = bx - x1;
    float bdy = by - y1;
    if (bdx * nx + bdy * ny < 0) {
        nx = -nx;
        ny = -ny;
    }
    
    float dist = bdx * nx + bdy * ny;
    if (dist < ball.r && dist > -ball.r) {
        float dot = bdx * (lx/len) + bdy * (ly/len);
        if (dot > 0 && dot < len) {
            bx += nx * (ball.r - dist);
            by += ny * (ball.r - dist);
            
            float vDot = bvx * nx + bvy * ny;
            if (vDot < 0) {
                bvx = bvx - 1.5f * vDot * nx;
                bvy = bvy - 1.5f * vDot * ny;
                
                if (isFlipper) {
                    bvx += nx * 150.0f;
                    bvy += ny * 150.0f;
                    tone(BUZZER_PIN, 800, 30);
                }
            }
        }
    }
}

unsigned long lastTime = 0;
unsigned long stateTimer = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.05f) dt = 0.05f;
    
    if (state == STATE_INTRO) {
        display_clear();
        drawRect(0, 0, 128, 64, 1);
        drawRect(2, 2, 124, 60, 1);
        drawText(40, 20, "PINBALL");
        
        if ((now / 500) % 2 == 0) {
            drawText(20, 45, "CLICK TO START");
        }
        display_render();
        
        if (input_left() || input_right() || input_plunge()) {
            tone(BUZZER_PIN, 800, 100);
            delay(200);
            resetGame();
        }
        return;
    }
    
    if (state == STATE_GAMEOVER) {
        display_clear();
        drawText(35, 20, "GAME OVER");
        drawText(40, 30, score);
        drawText(15, 50, "CLICK TO RESTART");
        display_render();
        if ((input_left() || input_right() || input_plunge()) && now - stateTimer > 500) {
            resetGame();
            tone(BUZZER_PIN, 800, 100); delay(200);
        }
        return;
    }
    
    if (input_left()) leftFlipAngle = -0.5f; else leftFlipAngle = 0.5f;
    if (input_right()) rightFlipAngle = 3.64f; else rightFlipAngle = 2.64f;
    
    float lfx2 = lfx + cos(leftFlipAngle) * flipLen;
    float lfy2 = lfy + sin(leftFlipAngle) * flipLen;
    float rfx2 = rfx + cos(rightFlipAngle) * flipLen;
    float rfy2 = rfy + sin(rightFlipAngle) * flipLen;
    
    if (plunging) {
        if (input_plunge()) {
            ball.vy = 100.0f;
        } else if (ball.vy > 0) {
            ball.vy = -350.0f;
            tone(BUZZER_PIN, 400, 100);
            plunging = false;
        }
    } else {
        int steps = 3;
        float subDt = dt / steps;
        for(int s=0; s<steps; s++) {
            ball.vy += 150.0f * subDt;
            
            ball.x += ball.vx * subDt;
            ball.y += ball.vy * subDt;
            
            // Move target
            movingTarget.x += movingTarget.vx * subDt;
            if (movingTarget.x < 10) { movingTarget.x = 10; movingTarget.vx = -movingTarget.vx; }
            if (movingTarget.x > 110) { movingTarget.x = 110; movingTarget.vx = -movingTarget.vx; }
            
            if (ball.x < 2) { ball.x = 2; ball.vx *= -0.8f; }
            if (ball.x > 115) { ball.x = 115; ball.vx *= -0.8f; }
            if (ball.y < 2) { ball.y = 2; ball.vy *= -0.8f; }
            
            for(int i=0; i<NUM_BUMPERS; i++) {
                resolveCircleCollision(ball.x, ball.y, ball.vx, ball.vy, bumpers[i].x, bumpers[i].y, bumpers[i].r, 50);
            }
            
            // Resolve moving target collision
            resolveCircleCollision(ball.x, ball.y, ball.vx, ball.vy, movingTarget.x, movingTarget.y, movingTarget.r, 200);
            
            resolveLineCollision(ball.x, ball.y, ball.vx, ball.vy, lfx, lfy, lfx2, lfy2, input_left());
            resolveLineCollision(ball.x, ball.y, ball.vx, ball.vy, rfx, rfy, rfx2, rfy2, input_right());
            
            resolveLineCollision(ball.x, ball.y, ball.vx, ball.vy, 20, 45, 35, 55, true);
            resolveLineCollision(ball.x, ball.y, ball.vx, ball.vy, 108, 45, 93, 55, true);
            
            if (ball.y > 64) {
                balls--;
                tone(BUZZER_PIN, 100, 300);
                if (balls <= 0) {
                    state = STATE_GAMEOVER;
                    stateTimer = now;
                } else {
                    plunging = true;
                    ball.x = 120;
                    ball.y = 50;
                    ball.vx = 0;
                    ball.vy = 0;
                }
            }
        }
    }
    
    ball.vx *= 0.99f;
    ball.vy *= 0.99f;
    
    display_clear();
    
    drawLine(0, 0, 117, 0, 1);
    drawLine(0, 0, 0, 63, 1);
    drawLine(117, 0, 117, 63, 1);
    drawLine(127, 20, 127, 63, 1);
    drawLine(117, 20, 127, 20, 1);
    
    drawLine(20, 45, 35, 55, 1);
    drawLine(108, 45, 93, 55, 1);
    
    for(int i=0; i<NUM_BUMPERS; i++) {
        drawCircle(bumpers[i].x, bumpers[i].y, bumpers[i].r, 1);
        drawCircle(bumpers[i].x, bumpers[i].y, bumpers[i].r-2, 1);
    }
    
    // Draw Moving Target
    fillRect((int)movingTarget.x - 2, (int)movingTarget.y - 2, 5, 5, 1);
    
    drawLine((int)lfx, (int)lfy, (int)lfx2, (int)lfy2, 1);
    drawLine((int)rfx, (int)rfy, (int)rfx2, (int)rfy2, 1);
    
    drawCircle((int)ball.x, (int)ball.y, (int)ball.r, 1);
    
    drawText(2, 2, score);
    drawText(2, 12, balls);
    
    display_render();
}

}
