#include "pet_display.h"
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 menuDisplay;

namespace Pet {

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

void drawPet(int x, int y, int state, int animFrame) {
    // Basic blob shape
    fillRect(x + 4, y, 16, 16, WHITE);
    fillRect(x + 2, y + 2, 20, 12, WHITE);
    fillRect(x, y + 4, 24, 10, WHITE);
    
    // Eyes
    if (state == 0) { // Normal
        if (animFrame == 0) { // open
            fillRect(x + 6, y + 4, 4, 4, BLACK);
            fillRect(x + 14, y + 4, 4, 4, BLACK);
        } else { // blink
            drawLine(x + 6, y + 6, x + 9, y + 6, BLACK);
            drawLine(x + 14, y + 6, x + 17, y + 6, BLACK);
        }
        // Mouth
        drawLine(x + 10, y + 10, x + 13, y + 10, BLACK);
    } else if (state == 1) { // Eating
        fillRect(x + 6, y + 4, 4, 4, BLACK);
        fillRect(x + 14, y + 4, 4, 4, BLACK);
        if (animFrame == 0) {
            drawRect(x + 10, y + 10, 4, 4, BLACK);
        } else {
            drawLine(x + 10, y + 10, x + 13, y + 10, BLACK);
        }
    } else if (state == 2) { // Sleeping
        drawLine(x + 6, y + 6, x + 9, y + 6, BLACK);
        drawLine(x + 14, y + 6, x + 17, y + 6, BLACK);
        // Zzz
        if (animFrame == 1) {
            drawText(x + 22, y - 8, "z");
        }
    } else if (state == 3) { // Sad / Sick
        drawLine(x + 6, y + 4, x + 9, y + 6, BLACK);
        drawLine(x + 14, y + 6, x + 17, y + 4, BLACK);
        // Frown
        drawLine(x + 10, y + 12, x + 13, y + 10, BLACK);
    }
    
    // Legs
    if (animFrame == 0) {
        fillRect(x + 4, y + 16, 4, 4, WHITE);
        fillRect(x + 16, y + 16, 4, 4, WHITE);
    } else {
        fillRect(x + 2, y + 14, 4, 4, WHITE);
        fillRect(x + 18, y + 14, 4, 4, WHITE);
    }
}

void drawIcon(int x, int y, int type, bool selected) {
    if (selected) {
        drawRect(x, y, 16, 16, WHITE);
        drawRect(x-1, y-1, 18, 18, WHITE);
    } else {
        drawRect(x, y, 16, 16, WHITE);
    }
    
    if (type == 0) { // Food (Burger)
        fillRect(x + 4, y + 4, 8, 4, WHITE);
        drawLine(x + 4, y + 9, x + 11, y + 9, WHITE);
        fillRect(x + 4, y + 11, 8, 2, WHITE);
    } else if (type == 1) { // Play (Ball)
        drawRect(x + 4, y + 4, 8, 8, WHITE);
        drawLine(x + 8, y + 4, x + 8, y + 11, WHITE);
        drawLine(x + 4, y + 8, x + 11, y + 8, WHITE);
    } else if (type == 2) { // Clean (Broom/Brush)
        fillRect(x + 4, y + 8, 8, 6, WHITE);
        drawLine(x + 8, y + 2, x + 8, y + 8, WHITE);
    } else if (type == 3) { // Sleep (Moon)
        drawLine(x + 8, y + 4, x + 10, y + 2, WHITE);
        drawLine(x + 8, y + 12, x + 10, y + 14, WHITE);
        drawLine(x + 6, y + 4, x + 6, y + 12, WHITE);
    }
}

}
