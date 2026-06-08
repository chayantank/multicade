#include "racing_display.h"
#include <Wire.h>

extern Adafruit_SSD1306 menuDisplay;

namespace Racing {

void display_setup() {
    menuDisplay.setRotation(1); // Portrait (64x128)
}

void display_clear() {
    menuDisplay.clearDisplay();
}

void display_render() {
    menuDisplay.display();
}

void drawSprite(int x, int y, const uint8_t *bitmap, int w, int h) {
    menuDisplay.drawBitmap(x, y, bitmap, w, h, WHITE);
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

void fillRect(int x, int y, int w, int h, int color) {
    menuDisplay.fillRect(x, y, w, h, color);
}

void drawLine(int x0, int y0, int x1, int y1, int color) {
    menuDisplay.drawLine(x0, y0, x1, y1, color);
}

}
