#include "terraria_display.h"
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 menuDisplay;

namespace Terraria {

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

void drawBlock(int x, int y, int type) {
    if (type == 1) { // Dirt (solid filled)
        fillRect(x, y, 4, 4, 1);
        drawPixel(x+1, y+1, 0); // specks
        drawPixel(x+3, y+2, 0);
    } else if (type == 2) { // Stone (outline + diag)
        drawRect(x, y, 4, 4, 1);
        drawLine(x, y, x+3, y+3, 1);
    } else if (type == 3) { // Wood
        drawRect(x, y, 4, 4, 1);
        drawLine(x+1, y, x+1, y+3, 1);
    } else if (type == 4) { // Leaves
        fillRect(x, y, 4, 4, 1);
        drawPixel(x, y, 0); drawPixel(x+2, y, 0);
        drawPixel(x+1, y+1, 0); drawPixel(x+3, y+1, 0);
        drawPixel(x, y+2, 0); drawPixel(x+2, y+2, 0);
        drawPixel(x+1, y+3, 0); drawPixel(x+3, y+3, 0);
    }
}

}
