#ifndef LIGHTCYCLE_DISPLAY_H
#define LIGHTCYCLE_DISPLAY_H

#include <Arduino.h>

namespace Lightcycle {
    void display_setup();
    void display_clear();
    void display_render();
    void drawRect(int x, int y, int w, int h, int color);
    void fillRect(int x, int y, int w, int h, int color);
    void drawLine(int x0, int y0, int x1, int y1, int color);
    void drawCircle(int x, int y, int r, int color);
    void drawPixel(int x, int y, int color);
    bool getPixel(int x, int y);
    void drawText(int x, int y, const char* text);
    void drawText(int x, int y, int number);
}

#endif
