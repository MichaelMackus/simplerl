#ifndef RANDOM_H
#define RANDOM_H

#define MIN_CELLS 8
#define MAX_CELLS 12
#define MAX_RANDOM_RECURSION 100

#include "map.h"

typedef struct {
    int x;
    int y;
} Coords;

typedef struct {
    int w;
    int h;
} Dimensions;

typedef struct {
    Coords coords;  // starting at top left corner of box
    Dimensions dimensions;
} Box;

// seed our (P)RNG
void seed_random();

Coords random_passable_coords(Level *level);

// randomly generate tiles by filling dungeon with cells
// and then connecting via corridors
void randomly_fill_tiles(Level *level);

#endif
