#ifndef RL_LIB_PATH_H
#define RL_LIB_PATH_H

#include "map.h"

struct RL_path;
typedef struct RL_path RL_path_t;

typedef double (*RL_heuristic_f)(RL_coords_t node, RL_coords_t end);
typedef int    (*RL_passable_f) (RL_coords_t node, void *user_data);

// get straight line from a to b using Bresenham's
RL_path_t *rl_get_line(const RL_coords_t a, const RL_coords_t b);

// A* pathfinding function
// pass 0 to diagonal_distance to disable moving diagonally, else you probably want sqrt(2) (~1.4)
// pass NULL to heuristic to use Dijkstra's algorithm
RL_path_t *rl_find_path(const RL_coords_t start,
                      const RL_coords_t end,
                      const RL_map_t *map,
                      double diagonal_distance,
                      RL_heuristic_f heuristic);

// A* pathfinding function, using a callback function to check for passable tiles.
// pass 0 to diagonal_distance to disable moving diagonally, else you probably want sqrt(2) (~1.4)
// pass NULL to heuristic to use Dijkstra's algorithm
// is_passable should return 1 if the tile is passable, else 0
// user_data gets passed directly to is_passable as the second argument
RL_path_t *rl_find_path_cb(const RL_coords_t start,
                         const RL_coords_t end,
                         double diagonal_distance,
                         RL_heuristic_f heuristic,
                         RL_passable_f  is_passable,
                         void *user_data);


// free & clear the path
void rl_clear_path(RL_path_t *path);

// walk along the path once and return the coords if found
const RL_coords_t *rl_walk_path(RL_path_t *path);

// reset path back to start point
void rl_reset_path(RL_path_t *path);

// reverse the path
void rl_reverse_path(RL_path_t *path);

#endif
