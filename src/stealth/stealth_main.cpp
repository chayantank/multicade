#include "stealth_main.h"
#include "stealth_display.h"
#include "stealth_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Stealth {

float px = 10, py = 50;

struct Guard {
    float x, y;
    float angle;
    float turnSpeed;
};

const int MAX_GUARDS = 3;
Guard guards[MAX_GUARDS];

struct Wall {
    float x, y, w, h;
};

const int MAX_WALLS = 5;
Wall walls[MAX_WALLS];

int level = 1;
bool gameOver = false;
bool levelClear = false;
bool showIntro = true;

void loadLevel() {
    if (level == 1) {
        px = 10; py = 50;
        walls[0] = {30, 20, 10, 30};
        walls[1] = {70, 10, 10, 30};
        for(int i=2; i<MAX_WALLS; i++) walls[i] = {-100, -100, 0, 0};
        
        guards[0] = {50, 25, 0, 1.0f};
        guards[1] = {90, 40, 3.14f, -1.0f};
        for(int i=2; i<MAX_GUARDS; i++) guards[i] = {-100, -100, 0, 0};
    } else {
        px = 10; py = 10;
        walls[0] = {40, 0, 10, 40};
        walls[1] = {80, 24, 10, 40};
        for(int i=2; i<MAX_WALLS; i++) walls[i] = {-100, -100, 0, 0};
        
        guards[0] = {20, 40, 1.5f, 1.5f};
        guards[1] = {60, 20, 0, 2.0f};
        guards[2] = {100, 50, 3.14f, -1.5f};
    }
    gameOver = false;
    levelClear = false;
}

void setup() {
    display_setup();
    input_setup();
    level = 1;
    showIntro = true;
    loadLevel();
}

bool checkWallCollision(float tx, float ty) {
    if (tx < 0 || tx > 127 || ty < 0 || ty > 63) return true;
    for(int i=0; i<MAX_WALLS; i++) {
        if (walls[i].w > 0) {
            if (tx > walls[i].x && tx < walls[i].x + walls[i].w &&
                ty > walls[i].y && ty < walls[i].y + walls[i].h) {
                return true;
            }
        }
    }
    return false;
}

bool pointInTriangle(float px, float py, float x1, float y1, float x2, float y2, float x3, float y3) {
    float areaOrig = abs((x2-x1)*(y3-y1) - (x3-x1)*(y2-y1));
    float area1 = abs((x1-px)*(y2-py) - (x2-px)*(y1-py));
    float area2 = abs((x2-px)*(y3-py) - (x3-px)*(y2-py));
    float area3 = abs((x3-px)*(y1-py) - (x1-px)*(y3-py));
    return abs(area1 + area2 + area3 - areaOrig) < 0.1f;
}

unsigned long lastTime = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.05f) dt = 0.05f;
    
    if (showIntro) {
        display_clear();
        drawRect(0, 0, 128, 64, 1);
        drawRect(2, 2, 124, 60, 1);
        drawText(30, 20, "STEALTH");
        if ((now / 500) % 2 == 0) drawText(20, 45, "CLICK TO START");
        display_render();
        if (input_action()) {
            tone(BUZZER_PIN, 800, 100);
            delay(200);
            showIntro = false;
            loadLevel();
        }
        return;
    }
    
    if (gameOver) {
        display_clear();
        drawText(35, 20, "CAUGHT!");
        display_render();
        if (input_action()) loadLevel();
        return;
    }
    
    if (levelClear) {
        display_clear();
        drawText(30, 20, "AREA CLEARED");
        display_render();
        if (input_action()) {
            level++;
            loadLevel();
        }
        return;
    }
    
    int jx = input_x() - 2048;
    int jy = input_y() - 2048;
    
    float nx = px;
    float ny = py;
    
    if (abs(jx) > 500) nx += (jx > 0 ? 1 : -1) * 30.0f * dt;
    if (abs(jy) > 500) ny += (jy > 0 ? 1 : -1) * 30.0f * dt;
    
    if (!checkWallCollision(nx, py)) px = nx;
    if (!checkWallCollision(px, ny)) py = ny;
    
    if (px > 115 && py > 50) {
        levelClear = true;
        tone(BUZZER_PIN, 800, 100); delay(100); tone(BUZZER_PIN, 1200, 100);
    }
    
    for(int i=0; i<MAX_GUARDS; i++) {
        if (guards[i].x < 0) continue;
        guards[i].angle += guards[i].turnSpeed * dt;
        
        float coneLength = 35.0f;
        float coneWidth = 0.6f;
        
        float p1x = guards[i].x;
        float p1y = guards[i].y;
        float p2x = guards[i].x + cos(guards[i].angle - coneWidth/2) * coneLength;
        float p2y = guards[i].y + sin(guards[i].angle - coneWidth/2) * coneLength;
        float p3x = guards[i].x + cos(guards[i].angle + coneWidth/2) * coneLength;
        float p3y = guards[i].y + sin(guards[i].angle + coneWidth/2) * coneLength;
        
        if (pointInTriangle(px, py, p1x, p1y, p2x, p2y, p3x, p3y)) {
            bool blocked = false;
            for(float t=0; t<1.0f; t+=0.1f) {
                float cx = guards[i].x + (px - guards[i].x) * t;
                float cy = guards[i].y + (py - guards[i].y) * t;
                for(int w=0; w<MAX_WALLS; w++) {
                    if (walls[w].w > 0 && cx > walls[w].x && cx < walls[w].x + walls[w].w && cy > walls[w].y && cy < walls[w].y + walls[w].h) {
                        blocked = true;
                    }
                }
            }
            if (!blocked) {
                gameOver = true;
                tone(BUZZER_PIN, 200, 500);
            }
        }
    }
    
    display_clear();
    
    drawRect(115, 50, 13, 14, 1);
    drawText(117, 53, "E");
    
    fillRect((int)px-1, (int)py-1, 3, 3, 1);
    
    for(int i=0; i<MAX_WALLS; i++) {
        if (walls[i].w > 0) {
            fillRect(walls[i].x, walls[i].y, walls[i].w, walls[i].h, 1);
        }
    }
    
    for(int i=0; i<MAX_GUARDS; i++) {
        if (guards[i].x < 0) continue;
        drawCircle(guards[i].x, guards[i].y, 2, 1);
        
        float coneLength = 35.0f;
        float coneWidth = 0.6f;
        float p2x = guards[i].x + cos(guards[i].angle - coneWidth/2) * coneLength;
        float p2y = guards[i].y + sin(guards[i].angle - coneWidth/2) * coneLength;
        float p3x = guards[i].x + cos(guards[i].angle + coneWidth/2) * coneLength;
        float p3y = guards[i].y + sin(guards[i].angle + coneWidth/2) * coneLength;
        
        drawLine(guards[i].x, guards[i].y, p2x, p2y, 1);
        drawLine(guards[i].x, guards[i].y, p3x, p3y, 1);
        drawLine(p2x, p2y, p3x, p3y, 1);
    }
    
    display_render();
}

}
