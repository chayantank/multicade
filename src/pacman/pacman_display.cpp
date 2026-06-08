#include "pacman_display.h"

namespace Pacman {

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void display_setup() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed in Pacman"));
    }
    display.setRotation(1); // Set to portrait (64x128)
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
}

void display_clear() {
    display.clearDisplay();
}

void display_render() {
    display.display();
}

void drawSprite(int x, int y, const uint8_t* bitmap, int w, int h) {
    display.drawBitmap(x, y, bitmap, w, h, WHITE);
}

void fillRect(int x, int y, int w, int h, uint16_t color) {
    display.fillRect(x, y, w, h, color);
}

void drawText(int x, int y, const char* text) {
    display.setCursor(x, y);
    display.print(text);
}

void drawText(int x, int y, int number) {
    display.setCursor(x, y);
    display.print(number);
}

void drawPixel(int x, int y, uint16_t color) {
    display.drawPixel(x, y, color);
}

}
