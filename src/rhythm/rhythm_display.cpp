#include "rhythm_display.h"
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 menuDisplay;

namespace Rhythm {

void display_setup() {}

void display_clear() {
    menuDisplay.clearDisplay();
}

void display_render() {
    menuDisplay.display();
}

void drawRect(int x, int y, int w, int h, int color) {
    menuDisplay.drawRect(x, y, w, h, color);
}

void fillRect(int x, int y, int w, int h, int color) {
    menuDisplay.fillRect(x, y, w, h, color);
}

void drawLine(int x0, int y0, int x1, int y1, int color) {
    menuDisplay.drawLine(x0, y0, x1, y1, color);
}

void drawText(int x, int y, const char* text) {
    menuDisplay.setTextSize(1);
    menuDisplay.setTextColor(WHITE);
    menuDisplay.setCursor(x, y);
    menuDisplay.print(text);
}

void drawText(int x, int y, int number) {
    menuDisplay.setTextSize(1);
    menuDisplay.setTextColor(WHITE);
    menuDisplay.setCursor(x, y);
    menuDisplay.print(number);
}

void drawLane(int laneIdx, int hitEffectTime) {
    int startX = 30 + laneIdx * 18;
    // Hit line
    drawRect(startX, 50, 16, 8, WHITE);
    
    // Lane lines
    drawLine(startX - 2, 0, startX - 2, 64, WHITE);
    if (laneIdx == 3) {
        drawLine(startX + 18, 0, startX + 18, 64, WHITE);
    }
    
    // Hit effect flash
    if (hitEffectTime > 0) {
        fillRect(startX, 50, 16, 8, WHITE);
    }
    
    // Arrows for lanes
    int cy = 54;
    int cx = startX + 8;
    if (laneIdx == 0) { // Left
        drawLine(cx+2, cy-2, cx-2, cy, !hitEffectTime);
        drawLine(cx+2, cy+2, cx-2, cy, !hitEffectTime);
    } else if (laneIdx == 1) { // Up
        drawLine(cx-2, cy+2, cx, cy-2, !hitEffectTime);
        drawLine(cx+2, cy+2, cx, cy-2, !hitEffectTime);
    } else if (laneIdx == 2) { // Down
        drawLine(cx-2, cy-2, cx, cy+2, !hitEffectTime);
        drawLine(cx+2, cy-2, cx, cy+2, !hitEffectTime);
    } else if (laneIdx == 3) { // Right
        drawLine(cx-2, cy-2, cx+2, cy, !hitEffectTime);
        drawLine(cx-2, cy+2, cx+2, cy, !hitEffectTime);
    }
}

void drawNote(int laneIdx, int y) {
    int startX = 30 + laneIdx * 18;
    fillRect(startX + 2, y, 12, 6, WHITE);
    
    // Checkered pattern
    menuDisplay.drawPixel(startX+4, y+2, BLACK);
    menuDisplay.drawPixel(startX+10, y+4, BLACK);
}

}
