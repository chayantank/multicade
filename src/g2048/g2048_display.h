#ifndef G2048_DISPLAY_H
#define G2048_DISPLAY_H

namespace G2048 {
    void display_setup();
    void display_clear();
    void display_render();
    void drawRect(int x, int y, int w, int h, int color);
    void fillRect(int x, int y, int w, int h, int color);
    void drawText(int x, int y, const char* text);
    void drawText(int x, int y, int number);
    void drawLine(int x0, int y0, int x1, int y1, int color);
    
    void drawTile(int r, int c, int val);
}

#endif
