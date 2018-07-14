#ifndef TILE_H
#define TILE_H

#define TILE_FLOOR      0
#define TILE_WALL       1
#define TILE_STAIR_DOWN 2
#define TILE_STAIR_UP   3
#define TILE_DOOR       4

#include "item.h"

typedef struct {
    Item *items;
    int type; // one of TILE consts
} Tile;

#endif
