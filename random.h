#ifndef RANDOM_H
#define RANDOM_H

#define MIN_CELLS 3
#define MAX_CELLS 8

#include "map.h"

typedef struct {
    int x;
    int y;
} Coords;

typedef struct {
    Coords coords; // starting at top left corner of box
    int w, h;      // dimensions w x h
} Box;

// seed our (P)RNG
void seed_random();

Coords random_coords(Level *level);
Coords random_passable_coords(Level *level);

// get random passable box with dimensions w x h
Box random_cell(Level *level, int w, int h);

#endif
