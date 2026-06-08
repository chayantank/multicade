# Multicade 40: The Ultimate ESP32 Arcade Collection

Welcome to **Multicade 40**, an massive collection of 40 classic, retro, and unique arcade games all packed into a single ESP32 microcontroller with a 128x64 OLED display.

## Hardware Requirements
- **ESP32** (Tested on ESP32 Dev Module, 240MHz)
- **128x64 OLED Display** (I2C SSD1306)
- **Analog Joystick** (VRX, VRY, SW)
- **Piezo Buzzer**

## Pin Mappings
Wire up your ESP32 according to these specific pin definitions. If you use different pins, you will need to update them in the source code (primarily `src/main.cpp` and specific game inputs).

| Component | ESP32 Pin | Description |
|---|---|---|
| **OLED SDA** | 21 | I2C Data |
| **OLED SCL** | 22 | I2C Clock |
| **Joystick VRX** | 34 | Analog X-axis (Left/Right) |
| **Joystick VRY** | 35 | Analog Y-axis (Up/Down) |
| **Joystick SW** | 27 | Action Button (Press / Hold) |
| **Buzzer** | 14 | PWM Audio Output |

## Global Navigation
* **Scroll Menu**: Move Joystick Up/Down to navigate the 40 games.
* **Select Game**: Press Joystick Button (SW) to start the selected game.
* **Return to Menu**: At **any** point during **any** game, hold down the Joystick Button (SW) for 3 seconds to trigger a clean exit back to the main menu without needing to reset the hardware.

## Games Included
1. Doom (Raycaster)
2. Tetris
3. Space Invaders
4. Pac-Man
5. Racing (Outrun style)
6. Breakout
7. Flappy Bird
8. Snake
9. Asteroids
10. Minesweeper
11. RPS (Rock Paper Scissors)
12. Catch the Egg
13. Simon Says
14. Maze Runner (with Braid Mazes)
15. Platformer (Mario style)
16. Starfighter (Defender style)
17. RPG (Zelda style)
18. Punch-Out
19. Bomber
20. Balloon Fight
21. Frogger / Hopper
22. Duck Shoot
23. Donkey Kong / Barrel Jump
24. MotoCross
25. Street Fighter (2D Brawler)
26. Lunar Lander
27. Paperboy
28. Sokoban
29. Lightcycle (Tron)
30. Bomberboy
31. Terraria (2D Sandbox)
32. Dino Run (Chrome Dino)
33. Rhythm Hero
34. Pocket Pet (Tamagotchi style)
35. Pixel Golf
36. Suika Drop
37. Missile Command
38. Q*bert
39. Stacker
40. 2048

## Installation & Compilation
This project uses **PlatformIO**. 
1. Clone the repository.
2. Open the project folder in VSCode + PlatformIO.
3. Build and Upload (`pio run -t upload`) to your ESP32.

Enjoy the ultimate retro experience on your ESP32!
