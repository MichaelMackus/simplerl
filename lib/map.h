#ifndef RL_MAP_H
#define RL_MAP_H

#include <stddef.h>

typedef struct {
    int x;
    int y;
} RL_Coords;
RL_Coords rl_coords(int x, int y);

struct _RL_Map;
typedef struct _RL_Map RL_Map;

/**
 * Create empty map (map full of impassable tiles). Can be used in the
 * rl_find_path function to find a path through the map structure. If
 * more flexibility is desired, it is suggested to use the
 * rl_find_path_cb function to supply a custom callback function to test
 * for passable tiles instead.
 *
 * The map is expected to start at an origin of 0,0 and extend in the
 * positive direction to width*height. The rl_*_passable functions are
 * used to declare/test if a coordinate on the map is passable. The map
 * does not handle negative coordinates.
 */
RL_Map* rl_create_map(size_t width, size_t height);

/**
 * Free the map.
 */
void rl_free_map(RL_Map *map);

/**
 * Return 1 if a tile is passable, else return 0.
 */
int rl_is_passable(const RL_Map* map, RL_Coords loc);

/**
 * Mark a tile as walkable on the map.
 */
void rl_set_passable(RL_Map* map, RL_Coords loc);

/**
 * Mark a tile as impassable on the map.
 */
void rl_set_impassable(RL_Map* map, RL_Coords loc);

#endif
