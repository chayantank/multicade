#ifndef INVADERS_SPRITES_H
#define INVADERS_SPRITES_H

#include <Arduino.h>

namespace Invaders {

#define SHIP_W 11
#define SHIP_H 8
const static uint8_t bmp_player[] PROGMEM = {
  0x04, 0x00, // .....#.....
  0x0E, 0x00, // ....###....
  0x0E, 0x00, // ....###....
  0x1F, 0x00, // ...#####...
  0x7F, 0xC0, // .#########.
  0xFF, 0xE0, // ###########
  0xFF, 0xE0, // ###########
  0xDB, 0x60  // ##.##.##.##
};

#define ALIEN_W 11
#define ALIEN_H 8
const static uint8_t bmp_alien_a[] PROGMEM = {
  0x20, 0x80, // ..#.....#..
  0x11, 0x00, // ...#...#...
  0x3f, 0x80, // ..#######..
  0x6d, 0xc0, // .##.###.##.
  0xff, 0xe0, // ###########
  0x52, 0xa0, // .#.#...#.#.
  0x40, 0x40, // .#.......#.
  0x11, 0x00  // ...#...#...
};

const static uint8_t bmp_alien_b[] PROGMEM = {
  0x0c, 0x00, // ....##.....
  0x1e, 0x00, // ...####....
  0x3f, 0x80, // ..#######..
  0x6d, 0xc0, // .##.###.##.
  0xff, 0xe0, // ###########
  0x12, 0x00, // ...#...#...
  0x2d, 0x80, // ..#.#.#.#..
  0x52, 0xa0  // .#.#...#.#.
};

#define ALIEN2_W 8
#define ALIEN2_H 8
const static uint8_t bmp_alien_c[] PROGMEM = {
  0x18, // ...##...
  0x3c, // ..####..
  0x7e, // .######.
  0xdb, // ##.##.##
  0xff, // ########
  0x24, // ..#..#..
  0x5a, // .#.#.#.#
  0xa5  // #.#..#.#
};

}

#endif
