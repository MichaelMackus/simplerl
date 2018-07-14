#ifndef TILE_H
#define TILE_H

#define TILE_FLOOR      0
#define TILE_WALL       1
#define TILE_WALL_SIDE  2
#define TILE_STAIR_DOWN 3
#define TILE_STAIR_UP   4
#define TILE_DOOR       5
#define TILE_DOOR_OPEN  6

#include "item.h"

typedef struct {
    Item *items;
    int type; // one of TILE consts
} Tile;

// return tile symbol for display
char tile_symbol(Tile tile);

// return 0 if impassable
int is_passable(Tile tile);

#endif
