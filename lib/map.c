#include "map.h"

#include <stdlib.h>

struct rl_map {
    size_t width;
    size_t height;
    char *tiles; // 2d array of tiles
};

typedef enum rl_tile {
    RL_TILE_STONE   = 0, //impassable by default
    RL_TILE_WALL    = 1, //impassable by default
    RL_TILE_ROOM    = 2, //passable by default
    RL_TILE_FLOOR   = 3, //passable by default
    RL_TILE_DOORWAY = 4  //passable by default
} rl_tile;

rl_map *rl_create_map(size_t width, size_t height)
{
    rl_map *map = malloc(sizeof(rl_map));

    if (map == NULL)
        return NULL;

    if (width < 0 || height < 0)
        return NULL;

    map->width = width;
    map->height = height;
    map->tiles = calloc(width * height, sizeof(char));

    if (map->tiles == NULL) {
        free(map);

        return NULL;
    }

    return map;
}

void rl_free_map(rl_map *map)
{
    if (map == NULL)
        return;

    if (map->tiles)
        free(map->tiles);

    free(map);
}

int rl_is_passable(const rl_map *map, rl_coords loc)
{
    if (map == NULL)
        return 0;

    if (loc.x < 0 || loc.y < 0)
        return 0;

    size_t index = loc.y * map->width + loc.x;
    if (index >= map->width * map->height)
        return 0;

    return map->tiles && map->tiles[index] & 0x80;
}

size_t rl_get_map_width(const rl_map *map)
{
    if (map == NULL)
        return 0;

    return map->width;
}

size_t rl_get_map_height(const rl_map *map)
{
    if (map == NULL)
        return 0;

    return map->height;
}

void rl_set_passable(rl_map *map, rl_coords loc)
{
    if (map == NULL || map->tiles == NULL)
        return;

    if (loc.x < 0 || loc.y < 0)
        return;

    size_t index = loc.y * map->width + loc.x;
    if (index < map->width * map->height) {
        map->tiles[index] |= 0x80;
    }
}

void rl_set_impassable(rl_map *map, rl_coords loc)
{
    if (map == NULL || map->tiles == NULL)
        return;

    if (loc.x < 0 || loc.y < 0)
        return;

    size_t index = loc.y * map->width + loc.x;
    if (index < map->width * map->height) {
        map->tiles[index] ^= 0x80;
    }
}

int rl_is_wall(const rl_map *map, rl_coords loc)
{
    if (map == NULL)
        return 0;

    if (loc.x < 0 || loc.y < 0)
        return 0;

    size_t index = loc.y * map->width + loc.x;
    if (index >= map->width * map->height)
        return 0;

    return map->tiles && map->tiles[index] & RL_TILE_WALL;
}

int rl_set_wall(const rl_map *map, rl_coords loc)
{
    if (map == NULL || map->tiles == NULL)
        return;

    if (loc.x < 0 || loc.y < 0)
        return;

    size_t index = loc.y * map->width + loc.x;
    if (index < map->width * map->height) {
        map->tiles[index] = RL_TILE_WALL;
    }
}

int rl_is_room(const rl_map *map, rl_coords loc)
{
    if (map == NULL)
        return 0;

    if (loc.x < 0 || loc.y < 0)
        return 0;

    size_t index = loc.y * map->width + loc.x;
    if (index >= map->width * map->height)
        return 0;

    return map->tiles && map->tiles[index] & RL_TILE_ROOM;
}

int rl_set_room(const rl_map *map, rl_coords loc)
{
    if (map == NULL || map->tiles == NULL)
        return;

    if (loc.x < 0 || loc.y < 0)
        return;

    size_t index = loc.y * map->width + loc.x;
    if (index < map->width * map->height) {
        map->tiles[index] = RL_TILE_ROOM | 0x80;
    }
}

int rl_is_doorway(const rl_map *map, rl_coords loc)
{
}

int rl_set_doorway(const rl_map *map, rl_coords loc)
{
}

/**
 * Get a tile type at a location.
 */
rl_tile rl_get_tile(const rl_map *map, rl_coords loc);

/**
 * Set a tile type at a location.
 */
rl_tile rl_set_tile(const rl_map *map, rl_coords loc, rl_tile tile);
