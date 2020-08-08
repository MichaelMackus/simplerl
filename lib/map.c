#include "map.h"

#include <stdlib.h>

struct rl_map {
    size_t width;
    size_t height;
    char *tiles; // 2d array of tiles
};

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

rl_tile rl_get_tile(const rl_map *map, rl_coords loc)
{
    if (map == NULL)
        return RL_TILE_INVALID;

    if (loc.x < 0 || loc.y < 0)
        return RL_TILE_INVALID;

    size_t index = loc.y * map->width + loc.x;
    if (index >= map->width * map->height)
        return RL_TILE_INVALID;

    if (!map->tiles)
        return RL_TILE_INVALID;

    if (map->tiles[index] & 0x80)
        return map->tiles[index] ^ 0x80;

    return map->tiles[index];
}

rl_tile rl_set_tile(const rl_map *map, rl_coords loc, rl_tile tile)
{
    if (map == NULL || map->tiles == NULL)
        return;

    if (loc.x < 0 || loc.y < 0)
        return;

    size_t index = loc.y * map->width + loc.x;
    if (index < map->width * map->height) {
        map->tiles[index] = tile;
        if (tile == RL_TILE_ROOM || tile == RL_TILE_PASSAGE || tile == RL_TILE_DOORWAY)
            rl_set_passable(map, loc);
    }
}

int rl_is_wall(const rl_map *map, rl_coords loc)
{
    return rl_get_tile(map, loc) == RL_TILE_WALL;
}

int rl_is_room(const rl_map *map, rl_coords loc)
{
    return rl_get_tile(map, loc) == RL_TILE_ROOM;
}

int rl_is_doorway(const rl_map *map, rl_coords loc)
{
    return rl_get_tile(map, loc) == RL_TILE_DOORWAY;
}
