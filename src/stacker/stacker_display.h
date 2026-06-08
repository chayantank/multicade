#ifndef STACKER_DISPLAY_H
#define STACKER_DISPLAY_H

namespace Stacker {
    void display_setup();
    void display_clear();
    void display_render();
    void drawRect(int x, int y, int w, int h, int color);
    void fillRect(int x, int y, int w, int h, int color);
    void drawText(int x, int y, const char* text);
    void drawText(int x, int y, int number);
}

#endif
