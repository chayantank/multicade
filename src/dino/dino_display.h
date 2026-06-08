#ifndef DINO_DISPLAY_H
#define DINO_DISPLAY_H

namespace Dino {
    void display_setup();
    void display_clear();
    void display_render();
    void drawRect(int x, int y, int w, int h, int color);
    void fillRect(int x, int y, int w, int h, int color);
    void drawLine(int x0, int y0, int x1, int y1, int color);
    void drawText(int x, int y, const char* text);
    void drawText(int x, int y, int number);
    void drawDino(int x, int y, int animFrame, bool ducking);
    void drawCactus(int x, int y, int type);
    void drawBird(int x, int y, int animFrame);
}

#endif
