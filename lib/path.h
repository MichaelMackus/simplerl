#ifndef RL_LIB_PATH_H
#define RL_LIB_PATH_H

typedef struct {
    int x;
    int y;
} RL_Coords;
RL_Coords rl_coords(int x, int y);

// get straight line from a to b using Bresenham's
const RL_Coords **rl_get_line(const RL_Coords a, const RL_Coords b, int maxLength);

// free the result from rl_get_line
void free_path(const RL_Coords **line, int maxLength);

#endif
