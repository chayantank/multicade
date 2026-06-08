#include "egg_display.h"
#include <Wire.h>

extern Adafruit_SSD1306 menuDisplay;

namespace Egg {

void display_setup() {
    menuDisplay.setRotation(1); // Portrait mode (64x128)
}

void display_clear() {
    menuDisplay.clearDisplay();
}

void display_render() {
    menuDisplay.display();
}

void drawBasket(int x, int y, int w, int h) {
    // U-shaped basket
    menuDisplay.drawLine(x, y, x, y + h, 1);
    menuDisplay.drawLine(x + w, y, x + w, y + h, 1);
    menuDisplay.drawLine(x, y + h, x + w, y + h, 1);
    // some weave pattern
    menuDisplay.drawLine(x + 2, y + 2, x + w - 2, y + h - 2, 1);
    menuDisplay.drawLine(x + w - 2, y + 2, x + 2, y + h - 2, 1);
}

void drawEgg(int x, int y) {
    // A simple oval
    menuDisplay.fillCircle(x, y, 3, 1);
    menuDisplay.drawPixel(x - 2, y - 4, 1);
    menuDisplay.drawPixel(x - 1, y - 4, 1);
    menuDisplay.drawPixel(x, y - 4, 1);
    menuDisplay.drawPixel(x + 1, y - 4, 1);
    menuDisplay.drawPixel(x + 2, y - 4, 1);
    
    menuDisplay.drawPixel(x - 1, y - 5, 1);
    menuDisplay.drawPixel(x, y - 5, 1);
    menuDisplay.drawPixel(x + 1, y - 5, 1);
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

void drawHeart(int x, int y) {
    menuDisplay.fillRect(x + 1, y, 2, 2, 1);
    menuDisplay.fillRect(x + 4, y, 2, 2, 1);
    menuDisplay.drawLine(x, y + 2, x + 6, y + 2, 1);
    menuDisplay.drawLine(x + 1, y + 3, x + 5, y + 3, 1);
    menuDisplay.drawLine(x + 2, y + 4, x + 4, y + 4, 1);
    menuDisplay.drawPixel(x + 3, y + 5, 1);
}

}
