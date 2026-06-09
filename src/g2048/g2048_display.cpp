#include "g2048_display.h"
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 menuDisplay;

namespace G2048 {

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

const int GRID_OFFSET_X = 64 - 30; // Center 60x60 grid
const int GRID_OFFSET_Y = 2;
const int TILE_SIZE = 15;

void drawTile(int r, int c, int val) {
    if (val == 0) return;
    int x = GRID_OFFSET_X + c * TILE_SIZE;
    int y = GRID_OFFSET_Y + r * TILE_SIZE;
    
    drawRect(x, y, TILE_SIZE, TILE_SIZE, 1);
    
    // Pattern or solid based on value
    if (val >= 8) {
        fillRect(x+1, y+1, TILE_SIZE-2, TILE_SIZE-2, 1);
        menuDisplay.setTextColor(BLACK);
    } else {
        menuDisplay.setTextColor(WHITE);
    }
    
    // Draw number (condensed)
    menuDisplay.setTextSize(1);
    if (val < 10) {
        menuDisplay.setCursor(x + 5, y + 4);
    } else if (val < 100) {
        menuDisplay.setCursor(x + 2, y + 4);
    } else if (val < 1000) {
        // Draw very small or just no space, 3 digits is tight in 15 pixels.
        menuDisplay.setCursor(x, y + 4);
    } else {
        // 1024, 2048
        // draw lines instead of text if it doesn't fit, or k
        menuDisplay.setCursor(x, y + 4);
        if (val == 1024) menuDisplay.print("1k");
        else if (val == 2048) menuDisplay.print("2k");
        else menuDisplay.print("4k");
        menuDisplay.setTextColor(WHITE);
        return;
    }
    menuDisplay.print(val);
    menuDisplay.setTextColor(WHITE); // Reset
}

}
