#ifndef SLICE_DISPLAY_H
#define SLICE_DISPLAY_H

#include <Adafruit_SSD1306.h>

namespace Slice {
    void display_setup();
    void display_clear();
    void display_render();
    void drawCircle(int x, int y, int r, int color);
    void drawLine(int x0, int y0, int x1, int y1, int color);
    void drawText(int x, int y, const char* str);
    void drawText(int x, int y, int num);
}

#endif
