#include "map.h"

#include <stdlib.h>

struct rl_map {
    size_t width;
    size_t height;
    rl_tile *tiles; // 2d array of tiles
    char *tile_flags; // 2d array of tile flags
};

#define RL_TILE_FLAG_PASSABLE 1

rl_map *rl_create_map(size_t width, size_t height)
{
    rl_map *map = malloc(sizeof(rl_map));

    if (map == NULL)
        return NULL;

    if (width < 0 || height < 0)
        return NULL;

    map->width = width;
    map->height = height;
    map->tiles = calloc(width * height, sizeof(rl_tile));
    map->tile_flags = calloc(width * height, sizeof(char));

    if (map->tiles == NULL || map->tile_flags == NULL) {
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

    if (map->tile_flags)
        free(map->tile_flags);

    free(map);
}

int rl_in_map_bounds(const rl_map *map, rl_coords loc)
{
    return loc.x < map->width && loc.x >= 0 && loc.y < map->height && loc.y >= 0;
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

    return map->tile_flags && map->tile_flags[index] & RL_TILE_FLAG_PASSABLE;
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
        map->tile_flags[index] |= RL_TILE_FLAG_PASSABLE;
    }
}

void rl_set_impassable(rl_map *map, rl_coords loc)
{
    if (map == NULL || map->tiles == NULL)
        return;

    if (loc.x < 0 || loc.y < 0)
        return;

    size_t index = loc.y * map->width + loc.x;
    if (index < map->width * map->height && map->tile_flags[index] & RL_TILE_FLAG_PASSABLE) {
        map->tile_flags[index] ^= RL_TILE_FLAG_PASSABLE;
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

char rl_wall_connections(const rl_map *map, rl_coords loc)
{
    char connection = 0;

    if (rl_is_wall(map, (rl_coords) { loc.x - 1, loc.y })) {
        connection |= RL_CONNECTION_L;
    }
    if (rl_is_wall(map, (rl_coords) { loc.x + 1, loc.y })) {
        connection |= RL_CONNECTION_R;
    }
    if (rl_is_wall(map, (rl_coords) { loc.x, loc.y - 1 })) {
        connection |= RL_CONNECTION_U;
    }
    if (rl_is_wall(map, (rl_coords) { loc.x, loc.y + 1 })) {
        connection |= RL_CONNECTION_D;
    }

    // diagonals
    if (rl_is_wall(map, (rl_coords) { loc.x - 1, loc.y - 1 })) {
        connection |= RL_CONNECTION_DIAG_UL;
    }
    if (rl_is_wall(map, (rl_coords) { loc.x + 1, loc.y + 1 })) {
        connection |= RL_CONNECTION_DIAG_UR;
    }
    if (rl_is_wall(map, (rl_coords) { loc.x - 1, loc.y - 1 })) {
        connection |= RL_CONNECTION_DIAG_DL;
    }
    if (rl_is_wall(map, (rl_coords) { loc.x + 1, loc.y + 1 })) {
        connection |= RL_CONNECTION_DIAG_DR;
    }

    return connection;
}

char rl_door_connections(const rl_map *map, rl_coords loc)
{
    char connection = 0;

    if (rl_is_doorway(map, (rl_coords) { loc.x - 1, loc.y })) {
        connection |= RL_CONNECTION_L;
    }
    if (rl_is_doorway(map, (rl_coords) { loc.x + 1, loc.y })) {
        connection |= RL_CONNECTION_R;
    }
    if (rl_is_doorway(map, (rl_coords) { loc.x, loc.y - 1 })) {
        connection |= RL_CONNECTION_U;
    }
    if (rl_is_doorway(map, (rl_coords) { loc.x, loc.y + 1 })) {
        connection |= RL_CONNECTION_D;
    }

    // diagonals
    if (rl_is_doorway(map, (rl_coords) { loc.x - 1, loc.y - 1 })) {
        connection |= RL_CONNECTION_DIAG_UL;
    }
    if (rl_is_doorway(map, (rl_coords) { loc.x + 1, loc.y + 1 })) {
        connection |= RL_CONNECTION_DIAG_UR;
    }
    if (rl_is_doorway(map, (rl_coords) { loc.x - 1, loc.y - 1 })) {
        connection |= RL_CONNECTION_DIAG_DL;
    }
    if (rl_is_doorway(map, (rl_coords) { loc.x + 1, loc.y + 1 })) {
        connection |= RL_CONNECTION_DIAG_DR;
    }

    return connection;
}

int rl_is_corner(const rl_map *map, rl_coords loc)
{
    char connections = rl_wall_connections(map, loc) | rl_door_connections(map, loc);

    if (connections == 0 || !rl_is_wall(map, loc)) {
        return 0;
    }

    return (connections & (RL_CONNECTION_L | RL_CONNECTION_U)) == (RL_CONNECTION_L | RL_CONNECTION_U) ||
           (connections & (RL_CONNECTION_L | RL_CONNECTION_D)) == (RL_CONNECTION_L | RL_CONNECTION_D) ||
           (connections & (RL_CONNECTION_R | RL_CONNECTION_U)) == (RL_CONNECTION_R | RL_CONNECTION_U) ||
           (connections & (RL_CONNECTION_R | RL_CONNECTION_D)) == (RL_CONNECTION_R | RL_CONNECTION_D);
}
