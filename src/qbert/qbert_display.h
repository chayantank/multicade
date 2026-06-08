#ifndef QBERT_DISPLAY_H
#define QBERT_DISPLAY_H

namespace Qbert {
    void display_setup();
    void display_clear();
    void display_render();
    void drawRect(int x, int y, int w, int h, int color);
    void fillRect(int x, int y, int w, int h, int color);
    void drawLine(int x0, int y0, int x1, int y1, int color);
    void drawPixel(int x, int y, int color);
    void drawText(int x, int y, const char* text);
    void drawText(int x, int y, int number);
    
    void getScreenPos(int r, int c, int &sx, int &sy);
    void drawCube(int sx, int sy, int state);
    void drawPlayer(int sx, int sy, int animOffset);
    void drawEnemy(int sx, int sy, int animOffset);
}

#endif
