#pragma once
#include <Adafruit_SSD1306.h>

namespace Egg {
    void display_setup();
    void display_clear();
    void display_render();
    void drawBasket(int x, int y, int w, int h);
    void drawEgg(int x, int y);
    void drawText(int x, int y, const char* text);
    void drawText(int x, int y, int number);
    void drawHeart(int x, int y);
    void drawBomb(int x, int y);
    void drawStar(int x, int y);
    void fillRect(int x, int y, int w, int h);
}
