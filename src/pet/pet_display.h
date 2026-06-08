#ifndef PET_DISPLAY_H
#define PET_DISPLAY_H

namespace Pet {
    void display_setup();
    void display_clear();
    void display_render();
    void drawRect(int x, int y, int w, int h, int color);
    void fillRect(int x, int y, int w, int h, int color);
    void drawLine(int x0, int y0, int x1, int y1, int color);
    void drawText(int x, int y, const char* text);
    void drawText(int x, int y, int number);
    
    void drawPet(int x, int y, int state, int animFrame);
    void drawIcon(int x, int y, int type, bool selected);
}

#endif
