# ESP32 OLED Tetris - Project Design Document

## Goal

Create a polished Tetris implementation for:

* ESP32 DevKit V1
* SSD1306 OLED 128x64
* Analog Joystick
* Passive Buzzer

The game should feel like a complete handheld console game rather than a technology demo.

---

# Hardware

## OLED

SSD1306 I2C 128x64

| OLED | ESP32  |
| ---- | ------ |
| VCC  | 3.3V   |
| GND  | GND    |
| SDA  | GPIO21 |
| SCL  | GPIO22 |

---

## Joystick

| Joystick | ESP32  |
| -------- | ------ |
| VCC      | 3.3V   |
| GND      | GND    |
| VRX      | GPIO34 |
| VRY      | GPIO35 |
| SW       | GPIO13 |

---

## Passive Buzzer

| Buzzer | ESP32  |
| ------ | ------ |
| SIG    | GPIO14 |
| GND    | GND    |

Use tone()/noTone() or LEDC PWM.

---

# Controls

Chosen mapping:

| Input          | Action           |
| -------------- | ---------------- |
| Left           | Move Piece Left  |
| Right          | Move Piece Right |
| Down           | Soft Drop        |
| Up             | Rotate Clockwise |
| Joystick Click | Hard Drop        |

Reasoning:

* Rotate is frequently used.
* Hard drop should require deliberate action.
* Prevents accidental piece destruction.

---

# Display Layout

OLED = 128x64

Recommended layout:

+-----------+------+
|           | NEXT |
|           |      |
|           | []   |
|           |[][]  |
|           |      |
|           |SCR   |
|           |1234  |
+-----------+------+

Board occupies left side.

Preview and score occupy right side.

---

# Board Dimensions

Logical board:

Width = 10
Height = 20

Classic Tetris dimensions.

---

# Pixel Geometry

Recommended:

Block Width = 5 pixels
Block Height = 3 pixels

Board:

10 × 5 = 50 px

20 × 3 = 60 px

Fits perfectly inside 128x64 display.

---

# Tetrominoes

Required:

I
O
T
S
Z
J
L

Store as:

pieces[7][4][4]

or

pieces[7][4]

bit-packed representation.

---

# Game State

Board:

uint8_t board[20][10];

Current Piece:

struct Piece
{
uint8_t type;
int8_t x;
int8_t y;
uint8_t rotation;
};

Next Piece:

Piece nextPiece;

---

# Spawn Rules

Spawn:

x = 3
y = 0

If collision occurs immediately:

GAME OVER

---

# Rotation

Clockwise only.

Use:

(rotation + 1) % 4

Implement simple wall kick:

Attempt:

1. Normal rotate
2. Shift left
3. Shift right

If all fail:

Cancel rotation.

---

# Collision Rules

Check:

* Left wall
* Right wall
* Bottom wall
* Existing blocks

before any movement.

---

# Falling Logic

Base drop interval:

700 ms

Level progression:

Level 1 = 700 ms
Level 2 = 600 ms
Level 3 = 500 ms
Level 4 = 400 ms
Level 5 = 300 ms

Minimum:

100 ms

---

# Hard Drop

On joystick click:

While no collision:
move down

Then:

lock piece immediately

Award:

+2 points per row dropped

---

# Soft Drop

While holding DOWN:

Increase fall speed.

Award:

+1 point per cell dropped

---

# Line Clear

Detect full row:

for row:
if all cells occupied:
remove row

Shift rows above downward.

---

# Scoring

Single Line = 100

Double Line = 300

Triple Line = 500

Tetris (4) = 800

Modern enough while remaining simple.

---

# Level Progression

Every:

10 lines cleared

Increase level.

Increase speed.

---

# Next Piece Preview

Display next tetromino.

Recommended area:

Top-right corner.

4x4 preview box.

---

# Ghost Piece

Strongly recommended.

Draw landing position using:

drawRect()

instead of

fillRect()

Benefits:

* Makes hard drop usable.
* Looks professional.
* Very little CPU cost.

---

# Sound Design

Move:
Short click

Rotate:
Higher click

Hard Drop:
Descending sweep

Line Clear:
Short melody

Level Up:
Ascending melody

Game Over:
Descending tones

Keep sounds under 150 ms.

Avoid blocking delays.

---

# Game Over Screen

Display:

GAME OVER

Score: XXXX

High Score: XXXX

Click to Restart

---

# High Score Storage

ESP32:

Preferences library

Store:

highscore

under namespace:

"tetris"

Persistence survives reboot.

---

# Recommended File Structure

src/
├── main.cpp
├── game.cpp
├── game.h
├── display.cpp
├── display.h
├── input.cpp
├── input.h
├── sound.cpp
├── sound.h

Keep rendering, input and gameplay separate.

---

# Nice Future Features

1. Hold Piece
2. Ghost Piece
3. High Score
4. Pause Screen
5. Difficulty Modes
6. Tiny Line-Clear Animation
7. Statistics Screen
8. Marathon Mode
9. Sprint Mode (40 lines)
10. WiFi Leaderboard

---

# Performance Expectations

ESP32 Usage:

RAM:
< 10 KB

Flash:
< 100 KB

FPS:
30-60 FPS

The SSD1306 display update will be the bottleneck, not the ESP32.

---

# Development Order

1. OLED Rendering
2. Board Data Structure
3. Falling Piece
4. Movement
5. Collision
6. Rotation
7. Locking
8. Line Clear
9. Score
10. Game Over
11. Next Piece
12. Ghost Piece
13. Sounds
14. High Score
15. Polish
