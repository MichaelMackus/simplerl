#ifndef RL_LIB_PATH_H
#define RL_LIB_PATH_H

typedef struct {
    int x;
    int y;
} Coords;
Coords xy(int x, int y);

// get straight line from a to b using Bresenham's
const Coords **get_line(const Coords a, const Coords b, int maxLength);

// free the result from get_line
void free_path(const Coords **line, int maxLength);

#endif
