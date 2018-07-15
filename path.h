#ifndef PATH_H
#define PATH_H

#include "map.h"

typedef struct {
    int x;
    int y;
} Coords;

// return 1 if path between points is walkable, otherwise 0
int find_path(const Coords start, const Coords end, const Level *level);

#endif
