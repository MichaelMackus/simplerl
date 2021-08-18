#ifndef RL_MAP_H
#define RL_MAP_H

#include <stddef.h>

typedef struct rl_coords {
    int x;
    int y;
} rl_coords;

typedef enum rl_tile {
    RL_TILE_BLOCK = 0,   //impassable by default
    RL_TILE_WALL,        //impassable by default
    RL_TILE_ROOM,        //passable by default
    RL_TILE_PASSAGE,     //passable by default
    RL_TILE_DOORWAY,     //passable by default

    RL_TILE_INVALID = -1 //impassable by default
} rl_tile;

struct rl_map;
typedef struct rl_map rl_map;

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
rl_map *rl_create_map(size_t width, size_t height);

/**
 * Free the map.
 */
void rl_free_map(rl_map *map);

/**
 * Return 1 if a tile is passable, else return 0.
 */
int rl_is_passable(const rl_map *map, rl_coords loc);

/**
 * Return map width
 */
size_t rl_get_map_width(const rl_map *map);

/**
 * Return map height
 */
size_t rl_get_map_height(const rl_map *map);

/**
 * Mark a tile as walkable on the map.
 */
void rl_set_passable(rl_map *map, rl_coords loc);

/**
 * Mark a tile as impassable on the map.
 */
void rl_set_impassable(rl_map *map, rl_coords loc);

/**
 * Get a tile type at a location.
 */
rl_tile rl_get_tile(const rl_map *map, rl_coords loc);

/**
 * Set a tile type at a location.
 */
rl_tile rl_set_tile(const rl_map *map, rl_coords loc, rl_tile tile);

/**
 * Return 1 if a tile is a wall.
 */
int rl_is_wall(const rl_map *map, rl_coords loc);

/**
 * Return 1 if a tile is a room tile.
 */
int rl_is_room(const rl_map *map, rl_coords loc);

/**
 * Return 1 if a tile is a doorway tile.
 */
int rl_is_doorway(const rl_map *map, rl_coords loc);

/**
 * Return 1 if a tile is a wall and is on a corner surrounded by walls/doors.
 * TODO: this does not count diagonals.
 */
int rl_is_corner(const rl_map *map, rl_coords loc);

/**
 * Wall connection definitions - these are for the resulting bitmask from
 * rl_wall_connection
 */
#define RL_CONNECTION_R       0b00000001
#define RL_CONNECTION_L       0b00000010
#define RL_CONNECTION_D       0b00000100
#define RL_CONNECTION_U       0b00001000
#define RL_CONNECTION_DIAG_UL 0b00010000
#define RL_CONNECTION_DIAG_UR 0b00100000
#define RL_CONNECTION_DIAG_DL 0b01000000
#define RL_CONNECTION_DIAG_DR 0b10000000

/**
 * Return >0 if a tile is connected to nearby walls.
 *
 * Return value is a byte mask containing one or more of RL_CONNECTION_ defines
 * or'ed together.  For example, for walls that are connecting to the current
 * point to the left and right, the value would be "0b00000011" or
 * "RL_CONNECTION_L | RL_CONNECTION_R".
 */
char rl_wall_connections(const rl_map *map, rl_coords loc);

/**
 * Returns >0 if a tile is connected to nearby doors.
 *
 * Works the same way as rl_wall_connections but for connecting doors instead of
 * walls.
 */
char rl_door_connections(const rl_map *map, rl_coords loc);

#endif
