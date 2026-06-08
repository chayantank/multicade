#pragma once
#include <Adafruit_SSD1306.h>

namespace Flappy {
    void display_setup();
    void display_clear();
    void display_render();
    void fillRect(int x, int y, int w, int h, int color);
    void drawRect(int x, int y, int w, int h, int color);
    void drawText(int x, int y, const char* text);
    void drawText(int x, int y, int number);
    void drawCircle(int x, int y, int r, int color);
}
