#include "map.h"

#include <stdlib.h>

struct _RL_Map {
    int width;
    int height;
    char *tiles; // 2d array of tiles, set to 1 if tile is passable
};

RL_Coords rl_coords(int x, int y)
{
    RL_Coords c;
    c.x = x;
    c.y = y;

    return c;
}

RL_Map* rl_create_map(int width, int height)
{
    RL_Map *map = malloc(sizeof(RL_Map));

    if (map == NULL)
        return NULL;

    if (width < 0 || height < 0)
        return NULL;

    map->width = width;
    map->height = height;
    map->tiles = calloc((size_t)width * (size_t)height, sizeof(char));

    if (map->tiles == NULL)
    {
        free(map);

        return NULL;
    }

    return map;
}

void rl_free_map(RL_Map *map)
{
    if (map == NULL)
        return;

    if (map->tiles)
        free(map->tiles);

    free(map);
}

int rl_is_passable(RL_Map* map, int x, int y)
{
    if (map == NULL || map->tiles == NULL)
        return 0;

    // TODO handle negative coordinate system?
    if (x < 0 || y < 0)
        return 0;

    int index = y*map->width + x;
    if (index < map->width * map->height && map->tiles[index])
        return 1;

    return 0;
}

void rl_set_passable(RL_Map* map, int x, int y)
{
    if (map == NULL || map->tiles == NULL)
        return;

    // TODO handle negative coordinate system?
    if (x < 0 || y < 0)
        return;

    int index = y * map->width + x;
    if (index < map->width * map->height) {
        map->tiles[index] = 1;
    }
}

void rl_set_impassable(RL_Map* map, int x, int y)
{
    if (map == NULL || map->tiles == NULL)
        return;

    // TODO handle negative coordinate system?
    if (x < 0 || y < 0)
        return;

    int index = y * map->width + x;
    if (index < map->width * map->height) {
        map->tiles[index] = 0;
    }
}
