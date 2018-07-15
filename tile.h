#ifndef TILE_H
#define TILE_H

#define TILE_NONE       0
#define TILE_CAVERN     1
#define TILE_FLOOR      2
#define TILE_WALL       3
#define TILE_WALL_SIDE  4
#define TILE_STAIR_DOWN 5
#define TILE_STAIR_UP   6
#define TILE_DOOR       7
#define TILE_DOOR_OPEN  8

#include "item.h"

typedef struct {
    Item *items;
    int type; // one of TILE consts
    int seen; // seen by player?
} Tile;

// return tile symbol for display
char tile_symbol(Tile tile);

// return 0 if impassable
int is_passable(Tile tile);

// empty tile constructor
// argument tileType should be one of TILE consts
Tile create_tile(int tileType);

#endif
