#ifndef RL_MAP_H
#define RL_MAP_H

typedef struct {
    int x;
    int y;
} RL_Coords;
RL_Coords rl_coords(int x, int y);

struct _RL_Map;
typedef struct _RL_Map RL_Map;

/**
 * Create empty map (map full of impassable tiles).
 */
RL_Map* rl_create_map(int width, int height);

/**
 * Free the map.
 */
void rl_free_map(RL_Map *map);

/**
 * Pass a custom callback that will be used to check for a passable tile. This
 * can be used in pathfinding to check for passable tiles (and overrides any
 * calls to rl_set_passable/rl_set_impassable).
 *
 * The first argument takes a function pointer that should return 1 if the
 * coords are passable or 0 if impassable. The user_data is sent untouched and
 * this func will be called on each call of rl_is_passable.
 */
void rl_set_passable_func(RL_Map *map,
                          int (*is_passable)(RL_Coords coords, void *user_data),
                          void *user_data);

/**
 * Return 1 if a tile is passable, else return 0.
 */
int rl_is_passable(RL_Map* map, int x, int y);

/**
 * Mark a tile as walkable on the map.
 */
void rl_set_passable(RL_Map* map, int x, int y);

/**
 * Mark a tile as impassable on the map.
 */
void rl_set_impassable(RL_Map* map, int x, int y);

#endif
