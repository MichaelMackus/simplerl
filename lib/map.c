#include "map.h"

#include <stdlib.h>

struct _RL_Map {
    int width;
    int height;
    char *tiles; // 2d array of tiles, set to 1 if tile is passable; only used if passable_func is null
    int (*passable_func)(RL_Coords coords, void *user_data);
    void *user_data; // only used if passable_func
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

    // TODO don't alloc until we need to, we might send a passable func
    map->width = width;
    map->height = height;
    map->tiles = calloc((size_t)width * (size_t)height, sizeof(char));
    map->passable_func = NULL;

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

void rl_set_passable_func(RL_Map *map,
                          int (*is_passable)(RL_Coords coords, void *user_data),
                          void *user_data)
{
    map->passable_func = is_passable;
    map->user_data = user_data;
}

int rl_is_passable(RL_Map* map, int x, int y)
{
    if (map == NULL)
        return 0;

    // TODO handle negative coordinate system?
    if (x < 0 || y < 0)
        return 0;

    int index = y*map->width + x;
    if (index >= map->width * map->height)
        return 0;

    // override with passable_func if defined
    if (map->passable_func)
        return map->passable_func(rl_coords(x, y), map->user_data);

    return map->tiles && map->tiles[index];
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
