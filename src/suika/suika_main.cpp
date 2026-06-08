#include "suika_main.h"
#include "suika_display.h"
#include "suika_input.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Suika {

extern int fruitRadii[]; // 3, 5, 8, 12, 16, 22, 30

struct Fruit {
    float x, y;
    float vx, vy;
    int level;
    bool active;
};

const int MAX_FRUITS = 40;
Fruit fruits[MAX_FRUITS];

float dropX = 64;
int nextFruitLevel = 0;
bool canDrop = true;
unsigned long lastDropTime = 0;

int score = 0;
bool gameOver = false;

const int BOX_LEFT = 20;
const int BOX_RIGHT = 108;
const int BOX_BOTTOM = 62;
const int BOX_TOP = 10; // line of death

void resetGame() {
    for(int i=0; i<MAX_FRUITS; i++) fruits[i].active = false;
    score = 0;
    dropX = 64;
    nextFruitLevel = random(0, 3);
    gameOver = false;
    canDrop = true;
}

void setup() {
    display_setup();
    input_setup();
    resetGame();
}

unsigned long lastTime = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;
    
    if (gameOver) {
        display_clear();
        drawText(30, 20, "OVERFLOW!");
        drawText(30, 30, "SCORE:");
        drawText(70, 30, score);
        display_render();
        if (input_action()) {
            tone(BUZZER_PIN, 800, 100); delay(200);
            resetGame();
        }
        return;
    }
    
    // Dropper Input
    if (input_left()) {
        dropX -= 40.0f * dt;
        if (dropX < BOX_LEFT + fruitRadii[nextFruitLevel]) dropX = BOX_LEFT + fruitRadii[nextFruitLevel];
    }
    if (input_right()) {
        dropX += 40.0f * dt;
        if (dropX > BOX_RIGHT - fruitRadii[nextFruitLevel]) dropX = BOX_RIGHT - fruitRadii[nextFruitLevel];
    }
    
    if (now - lastDropTime > 1000) canDrop = true;
    
    if (canDrop && input_action()) {
        // Spawn
        for(int i=0; i<MAX_FRUITS; i++) {
            if (!fruits[i].active) {
                fruits[i].active = true;
                fruits[i].x = dropX;
                fruits[i].y = 5;
                fruits[i].vx = 0;
                fruits[i].vy = 0;
                fruits[i].level = nextFruitLevel;
                break;
            }
        }
        canDrop = false;
        lastDropTime = now;
        nextFruitLevel = random(0, 3);
        tone(BUZZER_PIN, 1200, 30);
    }
    
    // Physics Step
    int iterations = 3;
    float subDt = dt / iterations;
    
    for(int iter = 0; iter < iterations; iter++) {
        for(int i=0; i<MAX_FRUITS; i++) {
            if (!fruits[i].active) continue;
            
            // Gravity
            fruits[i].vy += 150.0f * subDt;
            
            fruits[i].x += fruits[i].vx * subDt;
            fruits[i].y += fruits[i].vy * subDt;
            
            int r = fruitRadii[fruits[i].level];
            
            // Wall collisions
            if (fruits[i].x < BOX_LEFT + r) {
                fruits[i].x = BOX_LEFT + r;
                fruits[i].vx = -fruits[i].vx * 0.3f;
            }
            if (fruits[i].x > BOX_RIGHT - r) {
                fruits[i].x = BOX_RIGHT - r;
                fruits[i].vx = -fruits[i].vx * 0.3f;
            }
            if (fruits[i].y > BOX_BOTTOM - r) {
                fruits[i].y = BOX_BOTTOM - r;
                fruits[i].vy = -fruits[i].vy * 0.3f;
                fruits[i].vx *= 0.8f; // friction
            }
            
            // Inter-fruit collisions
            for(int j=i+1; j<MAX_FRUITS; j++) {
                if (!fruits[j].active) continue;
                
                float dx = fruits[j].x - fruits[i].x;
                float dy = fruits[j].y - fruits[i].y;
                float dist2 = dx*dx + dy*dy;
                int rj = fruitRadii[fruits[j].level];
                float minDist = r + rj;
                
                if (dist2 < minDist * minDist) {
                    float dist = sqrt(dist2);
                    if (dist == 0) dist = 0.1f;
                    
                    // Check merge
                    if (fruits[i].level == fruits[j].level && fruits[i].level < 6) {
                        // Merge!
                        fruits[i].level++;
                        fruits[i].x = (fruits[i].x + fruits[j].x) / 2.0f;
                        fruits[i].y = (fruits[i].y + fruits[j].y) / 2.0f;
                        fruits[i].vx = 0;
                        fruits[i].vy = 0;
                        fruits[j].active = false;
                        score += (fruits[i].level + 1) * 10;
                        tone(BUZZER_PIN, 400 + fruits[i].level*100, 50);
                        break; // Stop resolving this i, it just mutated
                    } else {
                        // Resolve overlap
                        float overlap = minDist - dist;
                        float nx = dx / dist;
                        float ny = dy / dist;
                        
                        // Push apart proportionally to mass (radius) roughly
                        float massI = r * r;
                        float massJ = rj * rj;
                        float totalMass = massI + massJ;
                        
                        float pushI = overlap * (massJ / totalMass);
                        float pushJ = overlap * (massI / totalMass);
                        
                        fruits[i].x -= nx * pushI;
                        fruits[i].y -= ny * pushI;
                        fruits[j].x += nx * pushJ;
                        fruits[j].y += ny * pushJ;
                        
                        // Simple velocity exchange (elasticish)
                        float dvx = fruits[j].vx - fruits[i].vx;
                        float dvy = fruits[j].vy - fruits[i].vy;
                        float dotProd = dvx*nx + dvy*ny;
                        
                        if (dotProd < 0) {
                            float restitution = 0.2f; // low bounce
                            float impulse = -(1.0f + restitution) * dotProd;
                            impulse /= (1.0f/massI + 1.0f/massJ);
                            
                            fruits[i].vx -= (impulse / massI) * nx;
                            fruits[i].vy -= (impulse / massI) * ny;
                            fruits[j].vx += (impulse / massJ) * nx;
                            fruits[j].vy += (impulse / massJ) * ny;
                        }
                    }
                }
            }
        }
    }
    
    // Check overflow
    bool anySettledHigh = false;
    for(int i=0; i<MAX_FRUITS; i++) {
        if (fruits[i].active && fruits[i].y - fruitRadii[fruits[i].level] < BOX_TOP) {
            if (abs(fruits[i].vx) < 5.0f && abs(fruits[i].vy) < 5.0f) {
                anySettledHigh = true;
            }
        }
    }
    if (anySettledHigh) {
        gameOver = true;
        tone(BUZZER_PIN, 100, 500);
    }
    
    // Render
    display_clear();
    
    // Box
    drawLine(BOX_LEFT, 10, BOX_LEFT, BOX_BOTTOM, 1);
    drawLine(BOX_RIGHT, 10, BOX_RIGHT, BOX_BOTTOM, 1);
    drawLine(BOX_LEFT, BOX_BOTTOM, BOX_RIGHT, BOX_BOTTOM, 1);
    
    // Danger line
    for(int x=BOX_LEFT; x<BOX_RIGHT; x+=4) {
        drawLine(x, BOX_TOP, x+2, BOX_TOP, 1); // dotted
    }
    
    // Fruits
    for(int i=0; i<MAX_FRUITS; i++) {
        if (fruits[i].active) {
            drawFruit((int)fruits[i].x, (int)fruits[i].y, fruits[i].level);
        }
    }
    
    // Dropper
    if (canDrop) {
        drawFruit((int)dropX, 5, nextFruitLevel);
    } else {
        drawLine(dropX-2, 5, dropX+2, 5, 1);
    }
    
    // Score
    drawText(2, 2, score);
    
    display_render();
}

}
