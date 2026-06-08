#ifndef BOMBERBOY_DISPLAY_H
#define BOMBERBOY_DISPLAY_H

namespace Bomberboy {
    void display_setup();
    void display_clear();
    void display_render();
    void drawRect(int x, int y, int w, int h, int color);
    void fillRect(int x, int y, int w, int h, int color);
    void drawLine(int x0, int y0, int x1, int y1, int color);
    void drawPixel(int x, int y, int color);
    void drawText(int x, int y, const char* text);
    void drawText(int x, int y, int number);
    void drawCircle(int x, int y, int r, int color);
    void fillCircle(int x, int y, int r, int color);
}

#endif
