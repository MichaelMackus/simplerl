#ifndef PATH_H
#define PATH_H

// TODO where to put this
#define MAX_WIDTH  80
#define MAX_HEIGHT 25

#include "tile.h"

typedef struct {
    int x;
    int y;
} Coords;

Coords xy(int x, int y);

// return 1 if path between points is walkable, otherwise 0
const Coords **find_path(const Coords start, const Coords end, const Tile **tiles);

// get straight line from a to b
const Coords **get_line(const Coords a, const Coords b);

// free the result from get_line and find_path
void free_path(const Coords **line);

#endif
