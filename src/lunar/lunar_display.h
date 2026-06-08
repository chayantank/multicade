#ifndef LUNAR_DISPLAY_H
#define LUNAR_DISPLAY_H

namespace Lunar {
    void display_setup();
    void display_clear();
    void display_render();
    void drawRect(int x, int y, int w, int h, int color);
    void fillRect(int x, int y, int w, int h, int color);
    void drawLine(int x0, int y0, int x1, int y1, int color);
    void drawPixel(int x, int y, int color);
    void drawText(int x, int y, const char* text);
    void drawText(int x, int y, int number);
    void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int color);
}

#endif
