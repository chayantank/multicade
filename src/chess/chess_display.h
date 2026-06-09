#ifndef CHESS_DISPLAY_H
#define CHESS_DISPLAY_H

namespace Chess {
    void display_setup();
    void display_clear();
    void display_render();
    
    void drawRect(int x, int y, int w, int h, int color);
    void fillRect(int x, int y, int w, int h, int color);
    void drawCircle(int x, int y, int r, int color);
    void drawLine(int x0, int y0, int x1, int y1, int color);
    void drawText(int x, int y, const char* str);
    void drawText(int x, int y, int num);
    void drawBitmap(int x, int y, const unsigned char bitmap[], int w, int h, int color);
}

#endif
