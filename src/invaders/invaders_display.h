#ifndef INVADERS_DISPLAY_H
#define INVADERS_DISPLAY_H

#include <Adafruit_SSD1306.h>

namespace Invaders {
    extern Adafruit_SSD1306 display;

    void display_setup();
    void display_clear();
    void display_render();
    void drawSprite(int x, int y, const uint8_t* bitmap, int w, int h);
    void drawRect(int x, int y, int w, int h, uint16_t color);
    void fillRect(int x, int y, int w, int h, uint16_t color);
    void drawText(int x, int y, const char* text);
    void drawText(int x, int y, int number);
}

#endif
