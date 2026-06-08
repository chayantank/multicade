#include "suika_display.h"
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 menuDisplay;

namespace Suika {

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

int fruitRadii[] = {3, 5, 8, 12, 16, 22, 30};

void drawFruit(int x, int y, int level) {
    if (level > 6) level = 6;
    int r = fruitRadii[level];
    
    drawCircle(x, y, r, 1);
    
    // Add some pattern inside to differentiate
    if (level == 0) { // Cherry
        drawLine(x, y-r, x+2, y-r-2, 1);
    } else if (level == 1) { // Strawberry
        menuDisplay.drawPixel(x-2, y-1, 1);
        menuDisplay.drawPixel(x+2, y+1, 1);
    } else if (level == 2) { // Grape
        drawCircle(x-2, y, 2, 1);
        drawCircle(x+2, y, 2, 1);
    } else if (level == 3) { // Dekopon
        drawRect(x-2, y-r-2, 4, 2, 1);
    } else if (level == 4) { // Apple
        drawLine(x, y-r, x, y-r+3, 1);
    } else if (level == 5) { // Peach
        drawLine(x, y-r, x, y, 1); // cleft
    } else if (level == 6) { // Watermelon
        drawLine(x-r+2, y, x+r-2, y, 1);
        drawLine(x, y-r+2, x, y+r-2, 1);
    }
}

}
