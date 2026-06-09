#ifndef GRAVITY_DISPLAY_H
#define GRAVITY_DISPLAY_H

#include <Adafruit_SSD1306.h>

namespace Gravity {
    void display_setup();
    void display_clear();
    void display_render();
    void drawRect(int x, int y, int w, int h, int color);
    void fillRect(int x, int y, int w, int h, int color);
    void drawLine(int x0, int y0, int x1, int y1, int color);
    void drawText(int x, int y, const char* str);
    void drawText(int x, int y, int num);
    void drawPixel(int x, int y, int color);
    void drawCircle(int x0, int y0, int r, int color);
}

#endif
