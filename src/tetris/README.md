# 🧱 Tetris on ESP32

A fully playable **Tetris** running on an **ESP32 DevKit V1** with a **128×64 SSD1306 OLED display**, **analog joystick**, and **passive buzzer** for sound effects. Features ghost piece, next piece preview, scoring, level progression, high score persistence, and wall-kick rotation.

**Author:** Chayan Tank

---

## 🎮 Gameplay

Classic Tetris with all 7 tetrominoes (I, O, T, S, Z, J, L), complete with:
- **Ghost piece** — outline shows where the piece will land
- **Next piece preview** — see what's coming next
- **Hard drop** — instant drop by clicking the joystick
- **Soft drop** — hold down for faster falling (+1 point per cell)
- **Wall-kick rotation** — rotate pieces even near walls
- **Scoring** — 100 / 300 / 500 / 800 for 1 / 2 / 3 / 4 lines
- **Level progression** — speed increases every 10 lines cleared
- **High score** — saved to flash, persists across reboots
- **Sound effects** — buzzer feedback for moves, clears, and game over

---

## 🧰 Components Required

| Component | Qty | Notes |
|---|---|---|
| ESP32 DevKit V1 (30-pin) | 1 | Any ESP32 with ADC on GPIO 34/35 |
| SSD1306 OLED Display (128×64, I2C) | 1 | I2C address `0x3C` |
| Analog Joystick Module (KY-023) | 1 | VRx, VRy, SW (click) |
| Passive Buzzer Module | 1 | For sound effects |
| Breadboard + Jumper Wires | — | For prototyping |
| Micro-USB Cable | 1 | Power and programming |

---

## 🔌 Wiring

### OLED Display (I2C)

| OLED | ESP32 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

### Joystick (KY-023)

| Joystick | ESP32 | Function |
|---|---|---|
| VRx | GPIO 34 | Left / Right |
| VRy | GPIO 35 | Up (Rotate) / Down (Soft Drop) |
| SW | GPIO 27 | Hard Drop (click) |
| VCC | 3.3V | Power |
| GND | GND | Ground |

### Passive Buzzer

| Buzzer | ESP32 |
|---|---|
| SIG | GPIO 14 |
| GND | GND |

### Wiring Diagram

```
ESP32 DevKit V1
┌──────────────────────┐
│                      │
│  3.3V ──────┬─────── OLED VCC
│             ├─────── Joystick VCC
│             │
│  GND ───────┼─────── OLED GND
│             ├─────── Joystick GND
│             └─────── Buzzer GND
│                      │
│  GPIO 21 (SDA) ───── OLED SDA
│  GPIO 22 (SCL) ───── OLED SCL
│                      │
│  GPIO 34 ──────────── Joystick VRx
│  GPIO 35 ──────────── Joystick VRy
│  GPIO 27 ──────────── Joystick SW
│  GPIO 14 ──────────── Buzzer SIG
│                      │
└──────────────────────┘
```

---

## 🕹️ Controls

| Input | Action |
|---|---|
| Joystick Left | Move piece left |
| Joystick Right | Move piece right |
| Joystick Up | Rotate clockwise |
| Joystick Down (hold) | Soft drop (+1 pt/cell) |
| Joystick Click | Hard drop (+2 pts/row) |

---

## 🛠️ Arduino IDE Setup

### 1. Board Support
Add ESP32 to Arduino IDE:
- **Preferences → Additional Board URLs:**
  `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- **Tools → Boards Manager** → Install **"esp32 by Espressif Systems"**

### 2. Libraries
Install via **Sketch → Include Library → Manage Libraries:**
- **Adafruit SSD1306**
- **Adafruit GFX Library** (installed automatically with SSD1306)

### 3. Board Settings
- **Board:** ESP32 Dev Module
- **Upload Speed:** 921600
- **Port:** Your ESP32's COM port

### 4. Upload
1. Open `esp32_working_tetris.ino` in Arduino IDE
2. Click **✓ Verify** to compile
3. Click **→ Upload** (hold BOOT button if needed)
4. Open Serial Monitor at **115200 baud** to see debug output

---

## 📂 Project Structure

```
esp32_working_tetris/
├── esp32_working_tetris.ino      # Main sketch — setup, loop, scene management
├── config.h        # Pin definitions, constants, game tuning
├── input.h/.cpp    # Joystick input with DAS auto-repeat
├── game.h/.cpp     # Core game logic, tetromino data, collision, scoring
├── display.h/.cpp  # SSD1306 rendering (board, pieces, HUD, screens)
├── sound.h/.cpp    # Buzzer sounds via LEDC PWM (non-blocking)
├── plan.md         # Original design document
└── README.md       # This file
```

---

## 🎯 Scoring & Levels

| Lines Cleared | Points |
|---|---|
| 1 (Single) | 100 |
| 2 (Double) | 300 |
| 3 (Triple) | 500 |
| 4 (Tetris) | 800 |

- **Soft drop:** +1 point per cell
- **Hard drop:** +2 points per row dropped
- **Level up** every 10 lines cleared
- **Speed:** 700ms (Lv.1) → 100ms (Lv.7+), decreasing by 100ms per level

---

## 📜 Credits

- Classic Tetris concept by Alexey Pajitnov (1985)
- ESP32 implementation by **Chayan Tank**
- Built with Arduino, Adafruit SSD1306, and Adafruit GFX

This project is for **educational and personal use only**.
