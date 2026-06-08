#include "SSD1306.h"
#include "constants.h"

namespace Doom {


Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    -1
);
} // namespace Doom
