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
