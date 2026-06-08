#ifndef TERRARIA_DISPLAY_H
#define TERRARIA_DISPLAY_H

namespace Terraria {
    void display_setup();
    void display_clear();
    void display_render();
    void drawRect(int x, int y, int w, int h, int color);
    void fillRect(int x, int y, int w, int h, int color);
    void drawLine(int x0, int y0, int x1, int y1, int color);
    void drawPixel(int x, int y, int color);
    void drawText(int x, int y, const char* text);
    void drawText(int x, int y, int number);
    
    // Draw 4x4 block patterns
    void drawBlock(int x, int y, int type);
}

#endif
