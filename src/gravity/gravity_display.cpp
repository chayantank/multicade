#include "gravity_display.h"

extern Adafruit_SSD1306 menuDisplay;

namespace Gravity {

void display_setup() {
    menuDisplay.setTextSize(1);
    menuDisplay.setTextColor(WHITE);
}

void display_clear() {
    menuDisplay.clearDisplay();
}

void display_render() {
    menuDisplay.display();
}

void drawRect(int x, int y, int w, int h, int color) {
    menuDisplay.drawRect(x, y, w, h, color == 1 ? WHITE : BLACK);
}

void fillRect(int x, int y, int w, int h, int color) {
    menuDisplay.fillRect(x, y, w, h, color == 1 ? WHITE : BLACK);
}

void drawLine(int x0, int y0, int x1, int y1, int color) {
    menuDisplay.drawLine(x0, y0, x1, y1, color == 1 ? WHITE : BLACK);
}

void drawText(int x, int y, const char* str) {
    menuDisplay.setCursor(x, y);
    menuDisplay.print(str);
}

void drawText(int x, int y, int num) {
    menuDisplay.setCursor(x, y);
    menuDisplay.print(num);
}

}
