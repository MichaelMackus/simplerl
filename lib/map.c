#include "map.h"

#include <stdlib.h>

struct rl_map {
    size_t width;
    size_t height;
    char *tiles; // 2d array of tiles, set to 1 if tile is passable
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

    return map->tiles && map->tiles[index];
}

void rl_set_passable(rl_map *map, rl_coords loc)
{
    if (map == NULL || map->tiles == NULL)
        return;

    if (loc.x < 0 || loc.y < 0)
        return;

    size_t index = loc.y * map->width + loc.x;
    if (index < map->width * map->height) {
        map->tiles[index] = 1;
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
        map->tiles[index] = 0;
    }
}
