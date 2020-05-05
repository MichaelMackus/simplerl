#ifndef RL_LIB_PATH_H
#define RL_LIB_PATH_H

#include "map.h"

struct _RL_Path;
typedef struct _RL_Path RL_Path;

// get straight line from a to b using Bresenham's
RL_Path *rl_get_line(const RL_Coords a, const RL_Coords b);

// A* pathfinding function
// pass 0 to diagonal distance to disable moving diagonally, else you probably want sqrt(2) (~1.4)
// pass NULL to heuristic_func to use Dijkstra's algorithm
RL_Path *rl_find_path(const RL_Coords start,
                      const RL_Coords end,
                      const RL_Map *map,
                      double diagonal_distance,
                      double (*heuristic_func)(RL_Coords node));

// free & clear the path
void rl_clear_path(RL_Path *path);

// walk along the path once and return the coords if found
const RL_Coords *rl_walk_path(RL_Path *path);

// reset path back to start point
void rl_reset_path(RL_Path *path);

// reverse the path
void rl_reverse_path(RL_Path *path);

#endif
