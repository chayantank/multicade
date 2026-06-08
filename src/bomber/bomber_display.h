#pragma once
#include <Adafruit_SSD1306.h>

namespace Bomber {
    void display_setup();
    void display_clear();
    void display_render();
    void drawRect(int x, int y, int w, int h, int color);
    void fillRect(int x, int y, int w, int h, int color);
    void drawCircle(int x, int y, int r, int color);
    void fillCircle(int x, int y, int r, int color);
    void drawLine(int x0, int y0, int x1, int y1, int color);
    void drawText(int x, int y, const char* text);
    void drawText(int x, int y, int number);
    void drawPixel(int x, int y, int color);
}
