#ifndef RL_LIB_PATH_H
#define RL_LIB_PATH_H

typedef struct {
    int x;
    int y;
} RL_Coords;
RL_Coords rl_coords(int x, int y);

struct _RL_Path;
typedef struct _RL_Path RL_Path;

// get straight line from a to b using Bresenham's
RL_Path *rl_get_line(const RL_Coords a, const RL_Coords b);

// free & clear the path
void rl_clear_path(RL_Path *path);

// walk along the path once and return the coords if found
const RL_Coords *rl_walk_path(RL_Path *path);

// reset path back to start point
void rl_reset_path(RL_Path *path);

// reverse the path
void rl_reverse_path(RL_Path *path);

#endif
