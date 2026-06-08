#include "qbert_display.h"
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 menuDisplay;

namespace Qbert {

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

void drawPixel(int x, int y, int color) {
    menuDisplay.drawPixel(x, y, color);
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

// Isometric sizing
const int TILE_W = 16;
const int TILE_H = 8;
const int CUBE_H = 10;
const int OFFSET_X = 64;
const int OFFSET_Y = 12;

void getScreenPos(int r, int c, int &sx, int &sy) {
    sx = OFFSET_X + (c - r) * (TILE_W / 2);
    sy = OFFSET_Y + (c + r) * (TILE_H / 2);
}

void drawCube(int sx, int sy, int state) {
    // Top face
    if (state == 0) { // Unvisited
        drawLine(sx, sy, sx + TILE_W/2, sy - TILE_H/2, 1);
        drawLine(sx + TILE_W/2, sy - TILE_H/2, sx + TILE_W, sy, 1);
        drawLine(sx + TILE_W, sy, sx + TILE_W/2, sy + TILE_H/2, 1);
        drawLine(sx + TILE_W/2, sy + TILE_H/2, sx, sy, 1);
    } else { // Visited - fill it with a pattern
        for(int i=0; i<TILE_W/2; i++) {
            drawLine(sx + i, sy - i/2, sx + i + TILE_W/2, sy + TILE_H/2 - i/2, 1);
        }
    }
    
    // Left face
    drawLine(sx, sy, sx, sy + CUBE_H, 1);
    drawLine(sx, sy + CUBE_H, sx + TILE_W/2, sy + TILE_H/2 + CUBE_H, 1);
    drawLine(sx + TILE_W/2, sy + TILE_H/2, sx + TILE_W/2, sy + TILE_H/2 + CUBE_H, 1);
    
    // Right face
    drawLine(sx + TILE_W, sy, sx + TILE_W, sy + CUBE_H, 1);
    drawLine(sx + TILE_W, sy + CUBE_H, sx + TILE_W/2, sy + TILE_H/2 + CUBE_H, 1);
}

void drawPlayer(int sx, int sy, int animOffset) {
    int px = sx + TILE_W/2;
    int py = sy - 6 - animOffset; // bounce up
    
    // Nose
    drawLine(px-4, py+2, px-2, py+4, 1);
    // Body
    drawRect(px-2, py-2, 6, 6, 1);
    // Legs
    menuDisplay.drawPixel(px-1, py+4, 1);
    menuDisplay.drawPixel(px+3, py+4, 1);
}

void drawEnemy(int sx, int sy, int animOffset) {
    int px = sx + TILE_W/2;
    int py = sy - 4 - animOffset;
    
    // Coiled spring look
    drawLine(px-3, py-3, px+3, py-3, 1);
    drawLine(px+3, py-3, px+1, py-1, 1);
    drawLine(px+1, py-1, px-1, py+1, 1);
    drawLine(px-1, py+1, px+3, py+3, 1);
}

}
