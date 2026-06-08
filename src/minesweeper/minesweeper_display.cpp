#include "minesweeper_display.h"
#include <Wire.h>

extern Adafruit_SSD1306 menuDisplay;

namespace Minesweeper {

void display_setup() {
    menuDisplay.setRotation(0); // Landscape mode (128x64)
}

void display_clear() {
    menuDisplay.clearDisplay();
}

void display_render() {
    menuDisplay.display();
}

void fillRect(int x, int y, int w, int h, int color) {
    menuDisplay.fillRect(x, y, w, h, color);
}

void drawRect(int x, int y, int w, int h, int color) {
    menuDisplay.drawRect(x, y, w, h, color);
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

void drawFlag(int x, int y) {
    // Flag is a simple triangle on a pole
    drawLine(x + 2, y + 6, x + 2, y + 1, 1); // pole
    drawLine(x + 2, y + 1, x + 5, y + 3, 1); // top
    drawLine(x + 5, y + 3, x + 2, y + 4, 1); // bottom
}

void drawMine(int x, int y) {
    // Mine is a little spiked ball
    menuDisplay.fillCircle(x + 3, y + 3, 2, 1);
    drawLine(x + 3, y, x + 3, y + 6, 1);
    drawLine(x, y + 3, x + 6, y + 3, 1);
    drawLine(x + 1, y + 1, x + 5, y + 5, 1);
    drawLine(x + 1, y + 5, x + 5, y + 1, 1);
}

}
