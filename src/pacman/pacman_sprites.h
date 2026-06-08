#ifndef PACMAN_SPRITES_H
#define PACMAN_SPRITES_H

#include <Arduino.h>

namespace Pacman {

// 8x8 sprites, 1 byte per row. MSB is leftmost pixel.
const static uint8_t bmp_pacman_closed[] PROGMEM = {
  0x3C, // ..####..
  0x7E, // .######.
  0xFF, // ########
  0xFF, // ########
  0xFF, // ########
  0xFF, // ########
  0x7E, // .######.
  0x3C  // ..####..
};

const static uint8_t bmp_pacman_left[] PROGMEM = {
  0x3C, // ..####..
  0x7E, // .######.
  0xFE, // #######.
  0xE0, // ###.....
  0xE0, // ###.....
  0xFE, // #######.
  0x7E, // .######.
  0x3C  // ..####..
};

const static uint8_t bmp_pacman_right[] PROGMEM = {
  0x3C, // ..####..
  0x7E, // .######.
  0x7F, // .#######
  0x07, // .....###
  0x07, // .....###
  0x7F, // .#######
  0x7E, // .######.
  0x3C  // ..####..
};

const static uint8_t bmp_pacman_up[] PROGMEM = {
  0x3C, // ..####..
  0x42, // .#....#.
  0x81, // #......#
  0x81, // #......#
  0xFF, // ########
  0xFF, // ########
  0x7E, // .######.
  0x3C  // ..####..
};

const static uint8_t bmp_pacman_down[] PROGMEM = {
  0x3C, // ..####..
  0x7E, // .######.
  0xFF, // ########
  0xFF, // ########
  0x81, // #......#
  0x81, // #......#
  0x42, // .#....#.
  0x3C  // ..####..
};

const static uint8_t bmp_ghost[] PROGMEM = {
  0x3C, // ..####..
  0x7E, // .######.
  0xFF, // ########
  0xA5, // #.#..#.# (eyes)
  0xFF, // ########
  0xFF, // ########
  0xDB, // ##.##.## (legs)
  0x81  // #......# 
};

const static uint8_t bmp_ghost_scared[] PROGMEM = {
  0x3C, // ..####..
  0x7E, // .######.
  0xFF, // ########
  0xDB, // ##.##.## (scared face)
  0xFF, // ########
  0xFF, // ########
  0x81, // #......# (legs alt)
  0xDB  // ##.##.##
};

}

#endif
