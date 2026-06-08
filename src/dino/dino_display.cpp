#include "dino_display.h"
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 menuDisplay;

namespace Dino {

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

void drawDino(int x, int y, int animFrame, bool ducking) {
    if (ducking) {
        // Ducking
        fillRect(x, y + 6, 16, 10, WHITE); // Body
        fillRect(x + 16, y + 6, 8, 6, WHITE); // Head
        menuDisplay.drawPixel(x + 20, y + 8, BLACK); // Eye
        
        // Legs
        if (animFrame == 0) {
            fillRect(x + 2, y + 16, 2, 4, WHITE);
            fillRect(x + 10, y + 16, 2, 2, WHITE);
        } else {
            fillRect(x + 2, y + 16, 2, 2, WHITE);
            fillRect(x + 10, y + 16, 2, 4, WHITE);
        }
    } else {
        // Standing
        fillRect(x + 6, y, 10, 8, WHITE); // Head
        fillRect(x + 2, y + 8, 12, 12, WHITE); // Body
        menuDisplay.drawPixel(x + 10, y + 2, BLACK); // Eye
        
        // Arm
        fillRect(x + 14, y + 10, 4, 2, WHITE);
        
        // Tail
        fillRect(x, y + 12, 2, 6, WHITE);
        
        // Legs
        if (animFrame == 0) {
            fillRect(x + 4, y + 20, 2, 4, WHITE);
            fillRect(x + 10, y + 20, 2, 2, WHITE);
        } else {
            fillRect(x + 4, y + 20, 2, 2, WHITE);
            fillRect(x + 10, y + 20, 2, 4, WHITE);
        }
    }
}

void drawCactus(int x, int y, int type) {
    if (type == 0) {
        // Small
        fillRect(x + 2, y, 4, 16, WHITE);
        fillRect(x, y + 4, 2, 4, WHITE);
        fillRect(x + 6, y + 6, 2, 4, WHITE);
    } else if (type == 1) {
        // Large
        fillRect(x + 3, y, 6, 24, WHITE);
        fillRect(x, y + 6, 3, 6, WHITE);
        fillRect(x + 9, y + 8, 3, 8, WHITE);
    } else if (type == 2) {
        // Group
        drawCactus(x, y + 8, 0);
        drawCactus(x + 8, y, 1);
        drawCactus(x + 18, y + 8, 0);
    }
}

void drawBird(int x, int y, int animFrame) {
    fillRect(x + 4, y + 4, 8, 4, WHITE); // Body
    fillRect(x, y + 2, 4, 2, WHITE); // Beak
    
    if (animFrame == 0) {
        // Wings Up
        fillRect(x + 6, y, 4, 4, WHITE);
    } else {
        // Wings Down
        fillRect(x + 6, y + 8, 4, 4, WHITE);
    }
}

}
