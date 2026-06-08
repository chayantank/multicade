#include "golf_main.h"
#include "golf_display.h"
#include "golf_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Golf {

int level = 1;
int strokes = 0;

float bx = 20, by = 30;
float vx = 0, vy = 0;
bool moving = false;

float angle = 0; // radians
float power = 0;
bool charging = false;
int powerDir = 1;

int holeX = 100, holeY = 30;
int holeR = 4;

struct Wall {
    int x, y, w, h;
};

const int MAX_WALLS = 5;
Wall walls[MAX_WALLS];
int numWalls = 0;

void loadLevel(int l) {
    moving = false;
    vx = 0; vy = 0;
    bx = 20; by = 30;
    
    if (l == 1) {
        holeX = 100; holeY = 30;
        numWalls = 1;
        walls[0] = {60, 10, 8, 44};
    } else if (l == 2) {
        holeX = 110; holeY = 10;
        bx = 10; by = 50;
        numWalls = 3;
        walls[0] = {40, 0, 8, 40};
        walls[1] = {80, 24, 8, 40};
        walls[2] = {10, 0, 118, 4}; // top boundary
    } else if (l == 3) {
        holeX = 20; holeY = 10;
        bx = 110; by = 50;
        numWalls = 4;
        walls[0] = {50, 20, 30, 20}; // center block
        walls[1] = {0, 0, 128, 4}; // top
        walls[2] = {0, 60, 128, 4}; // bottom
        walls[3] = {0, 0, 4, 64}; // left
    } else {
        // Random generated holes
        holeX = random(80, 120);
        holeY = random(10, 50);
        bx = 10; by = 30;
        numWalls = random(1, 5);
        for(int i=0; i<numWalls; i++) {
            walls[i].x = random(30, 70);
            walls[i].y = random(0, 40);
            walls[i].w = random(5, 15);
            walls[i].h = random(10, 30);
        }
    }
}

void setup() {
    display_setup();
    input_setup();
    level = 1;
    strokes = 0;
    loadLevel(level);
}

unsigned long lastTime = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    // Win check
    if (!moving && abs(bx - holeX) < holeR && abs(by - holeY) < holeR) {
        display_clear();
        drawText(30, 20, "HOLE IN ONE!"); // Actually just hole completed
        drawText(30, 30, "STROKES:");
        drawText(85, 30, strokes);
        display_render();
        tone(BUZZER_PIN, 1000, 100); delay(150); tone(BUZZER_PIN, 1500, 200);
        delay(2000);
        level++;
        strokes = 0;
        loadLevel(level);
        return;
    }
    
    // Physics
    if (moving) {
        // Apply friction
        vx *= 0.98f;
        vy *= 0.98f;
        
        if (abs(vx) < 1.0f && abs(vy) < 1.0f) {
            vx = 0; vy = 0;
            moving = false;
        }
        
        // Move X
        bx += vx * dt;
        // Collide X
        if (bx < 2 || bx > 126) {
            vx = -vx;
            bx += vx * dt;
            tone(BUZZER_PIN, 200, 20);
        }
        for(int i=0; i<numWalls; i++) {
            if (bx > walls[i].x && bx < walls[i].x + walls[i].w &&
                by > walls[i].y && by < walls[i].y + walls[i].h) {
                vx = -vx;
                bx += vx * dt;
                tone(BUZZER_PIN, 300, 20);
            }
        }
        
        // Move Y
        by += vy * dt;
        // Collide Y
        if (by < 2 || by > 62) {
            vy = -vy;
            by += vy * dt;
            tone(BUZZER_PIN, 200, 20);
        }
        for(int i=0; i<numWalls; i++) {
            if (bx > walls[i].x && bx < walls[i].x + walls[i].w &&
                by > walls[i].y && by < walls[i].y + walls[i].h) {
                vy = -vy;
                by += vy * dt;
                tone(BUZZER_PIN, 300, 20);
            }
        }
    } else {
        // Aiming
        int joyX = input_x() - 2048;
        int joyY = input_y() - 2048;
        
        if (abs(joyX) > 500 || abs(joyY) > 500) {
            angle = atan2(joyY, joyX);
        }
        
        if (input_action()) {
            charging = true;
            power += powerDir * 100.0f * dt;
            if (power > 200.0f) { power = 200.0f; powerDir = -1; }
            if (power < 0.0f) { power = 0.0f; powerDir = 1; }
        } else {
            if (charging) {
                // Shoot
                vx = cos(angle) * power * 2.0f;
                vy = sin(angle) * power * 2.0f;
                moving = true;
                charging = false;
                strokes++;
                tone(BUZZER_PIN, 800, 50);
                power = 0;
                powerDir = 1;
            }
        }
    }
    
    // Render
    display_clear();
    
    // Hole
    drawCircle(holeX, holeY, holeR, 1);
    fillRect(holeX-1, holeY-1, 2, 2, 0); // black center
    
    // Flag
    drawLine(holeX, holeY, holeX, holeY-10, 1);
    fillRect(holeX, holeY-10, 6, 4, 1);
    
    // Walls
    for(int i=0; i<numWalls; i++) {
        fillRect(walls[i].x, walls[i].y, walls[i].w, walls[i].h, 1);
    }
    
    // Ball
    drawPixel((int)bx, (int)by, 1);
    drawPixel((int)bx-1, (int)by, 1);
    drawPixel((int)bx+1, (int)by, 1);
    drawPixel((int)bx, (int)by-1, 1);
    drawPixel((int)bx, (int)by+1, 1);
    
    // Aiming line
    if (!moving && !charging) {
        drawLine(bx, by, bx + cos(angle)*10, by + sin(angle)*10, 1);
    }
    
    // Power bar
    if (charging) {
        drawRect(2, 58, 40, 4, 1);
        fillRect(2, 58, (int)(power / 5.0f), 4, 1);
    }
    
    // UI
    drawText(2, 2, "HOLE");
    drawText(28, 2, level);
    drawText(50, 2, "STRK:");
    drawText(80, 2, strokes);
    
    display_render();
}

}
