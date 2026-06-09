#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_system.h>
#include "barrel/barrel_main.h"
#include "motocross/motocross_main.h"
#include "fighter/fighter_main.h"
#include "lunar/lunar_main.h"
#include "paperboy/paperboy_main.h"
#include "sokoban/sokoban_main.h"
#include "lightcycle/lightcycle_main.h"
#include "bomberboy/bomberboy_main.h"
#include "terraria/terraria_main.h"
#include "dino/dino_main.h"
#include "rhythm/rhythm_main.h"
#include "pet/pet_main.h"
#include "golf/golf_main.h"
#include "suika/suika_main.h"
#include "missile/missile_main.h"
#include "qbert/qbert_main.h"
#include "stacker/stacker_main.h"
#include "g2048/g2048_main.h"
#include "slice/slice_main.h"
#include "pinball/pinball_main.h"
#include "lemmings/lemmings_main.h"
#include "slingshot/slingshot_main.h"
#include "gravity/gravity_main.h"
#include "towerdef/towerdef_main.h"
#include "bounce/bounce_main.h"
#include "stealth/stealth_main.h"
#include "qix/qix_main.h"
#include "fishing/fishing_main.h"

int active_game = 0;

// Hardware pins

#define JOY_Y 35
#define JOY_SW 27

namespace Doom { void setup(); void loop(); }
namespace Tetris { void setup(); void loop(); }
namespace Invaders { void setup(); void loop(); }
namespace Pacman { void setup(); void loop(); }
namespace Racing { void setup(); void loop(); }
namespace Breakout { void setup(); void loop(); }
namespace Flappy { void setup(); void loop(); }
namespace Snake { void setup(); void loop(); }
namespace Asteroids { void setup(); void loop(); }
namespace Minesweeper { void setup(); void loop(); }
namespace RPS { void setup(); void loop(); }
namespace Egg { void setup(); void loop(); }
namespace Simon { void setup(); void loop(); }
namespace Maze { void setup(); void loop(); }
namespace Platformer { void setup(); void loop(); }
namespace Starfighter { void setup(); void loop(); }
namespace RPG { void setup(); void loop(); }
namespace PunchOut { void setup(); void loop(); }
namespace Bomber { void setup(); void loop(); }
namespace Balloon { void setup(); void loop(); }
namespace Hopper { void setup(); void loop(); }
namespace DuckShoot { void setup(); void loop(); }
namespace Terraria { void setup(); void loop(); }
namespace Dino { void setup(); void loop(); }
namespace Rhythm { void setup(); void loop(); }
namespace Pet { void setup(); void loop(); }
namespace Golf { void setup(); void loop(); }
namespace Suika { void setup(); void loop(); }
namespace Missile { void setup(); void loop(); }
namespace Qbert { void setup(); void loop(); }
namespace Stacker { void setup(); void loop(); }
namespace G2048 { void setup(); void loop(); }
namespace Slice { void setup(); void loop(); }
namespace Pinball { void setup(); void loop(); }
namespace Lemmings { void setup(); void loop(); }
namespace Slingshot { void setup(); void loop(); }
namespace Gravity { void setup(); void loop(); }
namespace Towerdef { void setup(); void loop(); }
namespace Bounce { void setup(); void loop(); }
namespace Stealth { void setup(); void loop(); }
namespace Qix { void setup(); void loop(); }
namespace Fishing { void setup(); void loop(); }

Adafruit_SSD1306 menuDisplay(128, 64, &Wire, -1);

#define BUZZER_PIN 14

int menuSelection = 1; 
int menuScroll = 1;
bool lastSw = LOW;
unsigned long lastMoveTime = 0;
const int TOTAL_GAMES = 50;
const char* menuItems[TOTAL_GAMES] = {
    "1. Doom", "2. Tetris", "3. Space Invaders", "4. Pac-Man",
    "5. Racing", "6. Breakout", "7. Flappy Bird", "8. Snake",
    "9. Asteroids", "10. Minesweeper", "11. RPS", "12. Catch Egg",
    "13. Simon Says", "14. Maze Runner", "15. Platformer", "16. Starfighter",
    "17. RPG", "18. Punch-Out", "19. Bomber", "20. Balloon",
    "21. Hopper", "22. Duck Shoot", "23. Barrel Jump", "24. MotoCross",
    "25. Street Fighter", "26. Lunar Lander", "27. Paperboy", "28. Sokoban",
    "29. Lightcycle", "30. Bomberboy", "31. Terraria", "32. Dino Run",
    "33. Rhythm Hero", "34. Pocket Pet", "35. Pixel Golf", "36. Suika Drop",
    "37. Missile Cmd", "38. Q*bert", "39. Stacker", "40. 2048",
    "41. Fruit Slice", "42. Pinball", "43. Lemmings", "44. Slingshot",
    "45. Gravity Flip", "46. Tower Def", "47. Bounce Classic", "48. Stealth",
    "49. Qix Capture", "50. Fishing"
};

void setupMenu() {
    menuDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    menuDisplay.setRotation(0);
    menuDisplay.clearDisplay();
    pinMode(JOY_SW, INPUT_PULLUP);
    lastSw = digitalRead(JOY_SW);
}

void loopMenu() {
    int y = analogRead(JOY_Y);
    bool sw = digitalRead(JOY_SW);

    if (millis() - lastMoveTime > 200) {
        if (y < 1200) { 
            menuSelection--;
            if (menuSelection < 1) {
                menuSelection = TOTAL_GAMES;
                menuScroll = TOTAL_GAMES - 3;
            } else if (menuSelection < menuScroll) {
                menuScroll--;
            }
            tone(BUZZER_PIN, 800, 30);
            lastMoveTime = millis();
        } else if (y > 2800) { 
            menuSelection++;
            if (menuSelection > TOTAL_GAMES) {
                menuSelection = 1;
                menuScroll = 1;
            } else if (menuSelection > menuScroll + 3) {
                menuScroll++;
            }
            tone(BUZZER_PIN, 800, 30);
            lastMoveTime = millis();
        }
    }

    menuDisplay.clearDisplay();
    
    int titleY = 2 + (sin(millis() / 200.0) * 2);
    menuDisplay.setTextSize(2);
    menuDisplay.setTextColor(WHITE);
    menuDisplay.setCursor(10, titleY);
    menuDisplay.print("MULTICADE");

    menuDisplay.drawLine(10, titleY + 16, 118, titleY + 16, WHITE);

    int boxY = 24;
    
    menuDisplay.setTextSize(1);
    for(int i = menuScroll; i <= menuScroll + 3; i++) {
        if (i > TOTAL_GAMES) break;
        
        int row = i - menuScroll;
        int textY = boxY + row * 10;
        
        int len = strlen(menuItems[i-1]);
        int textX = (128 - (len * 6)) / 2;
        
        if (menuSelection == i) {
            menuDisplay.fillRect(textX - 12, textY - 1, (len * 6) + 24, 10, WHITE);
            menuDisplay.setTextColor(BLACK, WHITE);
            
            int arrowOffset = (millis() / 150) % 3;
            menuDisplay.setCursor(textX - 10 + arrowOffset, textY);
            menuDisplay.print(">");
            menuDisplay.setCursor(textX + (len * 6) + 4 - arrowOffset, textY);
            menuDisplay.print("<");
            
            menuDisplay.setCursor(textX, textY);
            menuDisplay.print(menuItems[i-1]);
        } else {
            menuDisplay.setTextColor(WHITE);
            menuDisplay.setCursor(textX, textY);
            menuDisplay.print(menuItems[i-1]);
        }
    }
    
    menuDisplay.display();

    if (sw == LOW && lastSw == HIGH) {
        tone(BUZZER_PIN, 1500, 150);
        
        for(int b=0; b<3; b++) {
            menuDisplay.invertDisplay(true);
            delay(100);
            menuDisplay.invertDisplay(false);
            delay(100);
        }
        
        menuDisplay.clearDisplay();
        menuDisplay.display();
        active_game = menuSelection;
        switch(active_game) {
            case 1: Doom::setup(); break;
            case 2: Tetris::setup(); break;
            case 3: Invaders::setup(); break;
            case 4: Pacman::setup(); break;
            case 5: Racing::setup(); break;
            case 6: Breakout::setup(); break;
            case 7: Flappy::setup(); break;
            case 8: Snake::setup(); break;
            case 9: Asteroids::setup(); break;
            case 10: Minesweeper::setup(); break;
            case 11: RPS::setup(); break;
            case 12: Egg::setup(); break;
            case 13: Simon::setup(); break;
            case 14: Maze::setup(); break;
            case 15: Platformer::setup(); break;
            case 16: Starfighter::setup(); break;
            case 17: RPG::setup(); break;
            case 18: PunchOut::setup(); break;
            case 19: Bomber::setup(); break;
            case 20: Balloon::setup(); break;
            case 21: Hopper::setup(); break;
            case 22: DuckShoot::setup(); break;
            case 23: Barrel::setup(); break;
            case 24: MotoCross::setup(); break;
            case 25: Fighter::setup(); break;
            case 26: Lunar::setup(); break;
            case 27: Paperboy::setup(); break;
            case 28: Sokoban::setup(); break;
            case 29: Lightcycle::setup(); break;
            case 30: Bomberboy::setup(); break;
            case 31: Terraria::setup(); break;
            case 32: Dino::setup(); break;
            case 33: Rhythm::setup(); break;
            case 34: Pet::setup(); break;
            case 35: Golf::setup(); break;
            case 36: Suika::setup(); break;
            case 37: Missile::setup(); break;
            case 38: Qbert::setup(); break;
            case 39: Stacker::setup(); break;
            case 40: G2048::setup(); break;
        }
        return;
    }
    lastSw = sw;
}

void setup() {
    Serial.begin(115200);
    delay(100);
    randomSeed(analogRead(0));

    if (active_game == 0) setupMenu();
    else {
        switch(active_game) {
            case 1: Doom::setup(); break;
            case 2: Tetris::setup(); break;
            case 3: Invaders::setup(); break;
            case 4: Pacman::setup(); break;
            case 5: Racing::setup(); break;
            case 6: Breakout::setup(); break;
            case 7: Flappy::setup(); break;
            case 8: Snake::setup(); break;
            case 9: Asteroids::setup(); break;
            case 10: Minesweeper::setup(); break;
            case 11: RPS::setup(); break;
            case 12: Egg::setup(); break;
            case 13: Simon::setup(); break;
            case 14: Maze::setup(); break;
            case 15: Platformer::setup(); break;
            case 16: Starfighter::setup(); break;
            case 17: RPG::setup(); break;
            case 18: PunchOut::setup(); break;
            case 19: Bomber::setup(); break;
            case 20: Balloon::setup(); break;
            case 21: Hopper::setup(); break;
            case 22: DuckShoot::setup(); break;
            case 23: Barrel::setup(); break;
            case 24: MotoCross::setup(); break;
            case 25: Fighter::setup(); break;
            case 26: Lunar::setup(); break;
            case 27: Paperboy::setup(); break;
            case 28: Sokoban::setup(); break;
            case 29: Lightcycle::setup(); break;
            case 30: Bomberboy::setup(); break;
            case 31: Terraria::setup(); break;
            case 32: Dino::setup(); break;
            case 33: Rhythm::setup(); break;
            case 34: Pet::setup(); break;
            case 35: Golf::setup(); break;
            case 36: Suika::setup(); break;
            case 37: Missile::setup(); break;
            case 38: Qbert::setup(); break;
            case 39: Stacker::setup(); break;
            case 40: G2048::setup(); break;
            case 41: Slice::setup(); break;
            case 42: Pinball::setup(); break;
            case 43: Lemmings::setup(); break;
            case 44: Slingshot::setup(); break;
            case 45: Gravity::setup(); break;
            case 46: Towerdef::setup(); break;
            case 47: Bounce::setup(); break;
            case 48: Stealth::setup(); break;
            case 49: Qix::setup(); break;
            case 50: Fishing::setup(); break;
        }
    }
}

void loop() {
    if (active_game == 0) {
        loopMenu();
    } else {
        // Global exit hook: Hold SW for 3s to return to menu
        static uint32_t global_hold_start = 0;
        static bool global_last_sw = HIGH;
        bool sw_state = digitalRead(JOY_SW);
        if (sw_state == LOW) {
            if (global_last_sw == HIGH) global_hold_start = millis();
            else if (millis() - global_hold_start >= 3000) {
                active_game = 0;
                ESP.restart();
            }
        }
        global_last_sw = sw_state;

        switch(active_game) {
            case 1: Doom::loop(); break;
            case 2: Tetris::loop(); break;
            case 3: Invaders::loop(); break;
            case 4: Pacman::loop(); break;
            case 5: Racing::loop(); break;
            case 6: Breakout::loop(); break;
            case 7: Flappy::loop(); break;
            case 8: Snake::loop(); break;
            case 9: Asteroids::loop(); break;
            case 10: Minesweeper::loop(); break;
            case 11: RPS::loop(); break;
            case 12: Egg::loop(); break;
            case 13: Simon::loop(); break;
            case 14: Maze::loop(); break;
            case 15: Platformer::loop(); break;
            case 16: Starfighter::loop(); break;
            case 17: RPG::loop(); break;
            case 18: PunchOut::loop(); break;
            case 19: Bomber::loop(); break;
            case 20: Balloon::loop(); break;
            case 21: Hopper::loop(); break;
            case 22: DuckShoot::loop(); break;
            case 23: Barrel::loop(); break;
            case 24: MotoCross::loop(); break;
            case 25: Fighter::loop(); break;
            case 26: Lunar::loop(); break;
            case 27: Paperboy::loop(); break;
            case 28: Sokoban::loop(); break;
            case 29: Lightcycle::loop(); break;
            case 30: Bomberboy::loop(); break;
            case 31: Terraria::loop(); break;
            case 32: Dino::loop(); break;
            case 33: Rhythm::loop(); break;
            case 34: Pet::loop(); break;
            case 35: Golf::loop(); break;
            case 36: Suika::loop(); break;
            case 37: Missile::loop(); break;
            case 38: Qbert::loop(); break;
            case 39: Stacker::loop(); break;
            case 40: G2048::loop(); break;
            case 41: Slice::loop(); break;
            case 42: Pinball::loop(); break;
            case 43: Lemmings::loop(); break;
            case 44: Slingshot::loop(); break;
            case 45: Gravity::loop(); break;
            case 46: Towerdef::loop(); break;
            case 47: Bounce::loop(); break;
            case 48: Stealth::loop(); break;
            case 49: Qix::loop(); break;
            case 50: Fishing::loop(); break;
        }
    }
}
