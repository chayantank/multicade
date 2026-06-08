#include "stacker_display.h"
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 menuDisplay;

namespace Stacker {

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

}
