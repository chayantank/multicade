#ifndef PACMAN_DISPLAY_H
#define PACMAN_DISPLAY_H

#include <Adafruit_SSD1306.h>

namespace Pacman {
    extern Adafruit_SSD1306 display;

    void display_setup();
    void display_clear();
    void display_render();
    void drawSprite(int x, int y, const uint8_t* bitmap, int w, int h);
    void fillRect(int x, int y, int w, int h, uint16_t color);
    void drawText(int x, int y, const char* text);
    void drawText(int x, int y, int number);
    void drawPixel(int x, int y, uint16_t color);
}

#endif
