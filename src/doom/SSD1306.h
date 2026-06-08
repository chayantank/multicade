#pragma once

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace Doom {


extern Adafruit_SSD1306 display;

inline void clearRect(int x, int y, int w, int h)
{
    display.fillRect(x, y, w, h, BLACK);
}
} // namespace Doom
