#ifndef RL_LIB_PATH_H
#define RL_LIB_PATH_H

#include "map.h"

struct rl_path;
typedef struct rl_path rl_path;

typedef double (*rl_heuristic_f)(rl_coords node, rl_coords end);
typedef int    (*rl_passable_f) (rl_coords node, void *user_data);

// get straight line from a to b using Bresenham's
rl_path *rl_get_line(const rl_coords a, const rl_coords b);

// get straight line from a to b using Manhattan distance (this
// resembles an "L" shape when there is a change in direction)
rl_path *rl_get_line_manhattan(const rl_coords a, const rl_coords b);

// A* pathfinding function
// pass 0 to diagonal_distance to disable moving diagonally, else you probably want sqrt(2) (~1.4)
// pass NULL to heuristic to use Dijkstra's algorithm
rl_path *rl_find_path(const rl_coords start,
                        const rl_coords end,
                        const rl_map *map,
                        double diagonal_distance,
                        rl_heuristic_f heuristic);

// A* pathfinding function, using a callback function to check for passable tiles.
// pass 0 to diagonal_distance to disable moving diagonally, else you probably want sqrt(2) (~1.4)
// pass NULL to heuristic to use Dijkstra's algorithm
// is_passable should return 1 if the tile is passable, else 0
// user_data gets passed directly to is_passable as the second argument
rl_path *rl_find_path_cb(const rl_coords start,
                           const rl_coords end,
                           double diagonal_distance,
                           rl_heuristic_f heuristic,
                           rl_passable_f  is_passable,
                           void *user_data);


// free & clear the path
void rl_clear_path(rl_path *path);

// walk along the path once and return the coords if found
const rl_coords *rl_walk_path(rl_path *path);

// reset path back to start point
void rl_reset_path(rl_path *path);

// reverse the path
void rl_reverse_path(rl_path *path);

#endif
