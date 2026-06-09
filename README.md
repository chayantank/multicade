# ESP32 Multicade - 50 Games in 1! 🎮

A massive compilation of 50 unique retro games and mini-games running entirely on a single ESP32 microcontroller with an SSD1306 OLED display.

## Hardware Requirements
- **ESP32 Development Board**
- **128x64 I2C SSD1306 OLED Display**
- **Analog Joystick** (with built-in push button)
- **Piezo Buzzer** (for audio)

## Pin Mappings
| Component | ESP32 Pin | Notes |
| :--- | :--- | :--- |
| OLED SDA | GPIO 21 | Standard I2C |
| OLED SCL | GPIO 22 | Standard I2C |
| Joystick VRX | GPIO 34 | Analog X-axis |
| Joystick VRY | GPIO 35 | Analog Y-axis |
| Joystick SW | GPIO 27 | Push button (Action/Exit) |
| Buzzer + | GPIO 14 | PWM Audio output |

## Controls
- **Joystick (VRX / VRY):** Move characters, navigate menus, control cursors.
- **Button (SW):** 
  - **Press:** Select game, Jump, Shoot, Action, Drop.
  - **Hold (3 seconds):** Exit any game and return to the main menu.
  - **Hold (0.5 seconds):** Toggle Action/Cursor mode in applicable games (like Terraria).

## Games Included
This project includes 50 distinct games ranging from classic arcade titles to physics-based puzzles and rhythm games.

1. Doom (Raycasting)
2. Tetris
3. Space Invaders
4. Pac-Man
5. Racing
6. Breakout
7. Flappy Bird
8. Snake
9. Asteroids
10. Minesweeper
11. RPS (Rock Paper Scissors)
12. Catch Egg
13. Simon Says
14. Maze Runner
15. Platformer
16. Starfighter
17. RPG
18. Punch-Out
19. Bomber
20. Balloon
21. Hopper
22. Duck Shoot
23. Barrel Jump
24. MotoCross
25. Street Fighter
26. Lunar Lander
27. Paperboy
28. Sokoban
29. Lightcycle
30. Bomberboy
31. Terraria
32. Dino Run
33. Rhythm Hero
34. Pocket Pet
35. Pixel Golf
36. Suika Drop
37. Missile Cmd
38. Q*bert
39. Stacker
40. 2048
41. Fruit Slice
42. Pinball
43. Lemmings
44. Slingshot
45. Gravity Flip
46. Tower Def
47. Bounce Classic
48. Stealth
49. Qix Capture
50. Fishing

## Building and Flashing
This project is built using **PlatformIO**. 

1. Install PlatformIO for VS Code or CLI.
2. Clone this repository.
3. Run `pio run -t upload` to build and flash to your ESP32.

### Memory Optimization
Despite containing 50 completely distinct game loops and physics engines, the architecture is highly optimized:
- **RAM Usage:** ~16%
- **Flash Usage:** ~37%

This leaves plenty of room for future expansions!
