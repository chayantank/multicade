#include "missile_display.h"
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 menuDisplay;

namespace Missile {

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

void drawCircle(int x0, int y0, int r, int color) {
    menuDisplay.drawCircle(x0, y0, r, color);
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

void drawCity(int x, int y) {
    fillRect(x, y, 12, 6, 1);
    fillRect(x+2, y-4, 4, 4, 1);
    fillRect(x+7, y-2, 3, 2, 1);
    
    // Windows
    menuDisplay.drawPixel(x+3, y-2, 0);
    menuDisplay.drawPixel(x+5, y-2, 0);
    menuDisplay.drawPixel(x+2, y+2, 0);
    menuDisplay.drawPixel(x+6, y+2, 0);
    menuDisplay.drawPixel(x+9, y+2, 0);
}

}
