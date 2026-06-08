#ifndef PACMAN_MAP_H
#define PACMAN_MAP_H

#include <Arduino.h>

namespace Pacman {

#define W 0 // Wall
#define D 1 // Dot
#define P 2 // Power Pellet
#define E 3 // Empty
#define G 4 // Ghost Door

const int MAP_W = 8;
const int MAP_H = 15;

const uint8_t PROGMEM default_map[MAP_H][MAP_W] = {
    {W,W,W,W,W,W,W,W}, // 0
    {W,D,D,D,D,D,D,W}, // 1
    {W,P,W,W,W,W,P,W}, // 2
    {W,D,D,D,D,D,D,W}, // 3
    {W,D,W,G,G,W,D,W}, // 4
    {E,D,W,E,E,W,D,E}, // 5
    {W,D,W,W,W,W,D,W}, // 6
    {W,D,D,D,D,D,D,W}, // 7
    {W,W,D,W,W,D,W,W}, // 8
    {W,D,D,D,D,D,D,W}, // 9
    {W,D,W,W,W,W,D,W}, // 10
    {W,D,D,D,D,D,D,W}, // 11
    {W,P,W,W,W,W,P,W}, // 12
    {W,D,D,D,D,D,D,W}, // 13
    {W,W,W,W,W,W,W,W}  // 14
};

}

#endif
