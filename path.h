#ifndef PATH_H
#define PATH_H

// TODO where to put this
#define MAX_WIDTH  80
#define MAX_HEIGHT 25

#include "tile.h"
#include "lib/path.h"

// return 1 if path between points is walkable, otherwise 0
const Coords **find_path(const Coords start, const Coords end, const Tile **tiles);

// simple lighting function to see if mob can see x and y
int can_see(Coords from, Coords to, Tile **tiles);

#endif
