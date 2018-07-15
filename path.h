#ifndef PATH_H
#define PATH_H

#include "map.h"

typedef struct {
    int x;
    int y;
} Coords;

Coords xy(int x, int y);

// return 1 if path between points is walkable, otherwise 0
int find_path(const Coords start, const Coords end, const Level *level);

// get straight line from a to b
const Coords **get_line(const Coords a, const Coords b);

// free the line from get_line
void free_line(const Coords **line);

#endif
