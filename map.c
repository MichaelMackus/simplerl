#include "map.h"
#include <stdlib.h>
#include <memory.h>

Level *create_level(int depth)
{
    Level *level;
    level = malloc(sizeof(Level));

    // check for OOM
    if (level == NULL)
        return NULL;

    // initialize mobs array
    level->mobs = malloc(sizeof(Mob*) * MAX_MOBS);
    memset(level->mobs, 0, sizeof(Mob*) * MAX_MOBS);

    // check for OOM
    if (level->mobs == NULL)
    {
        free(level);

        return NULL;
    }

    // initialize depth
    level->depth = depth;

    // initialize our NEXT & PREV pointers to NULL
    level->next = NULL;
    level->prev = NULL;

    // initialize basic tiles array
    // NOTE: initializing [y][x] since that is how C allocates
    // *row* would be first allocation, and *cols* second
    level->tiles = malloc(sizeof(Tile*) * MAX_HEIGHT);

    // check for OOM
    if (level->tiles == NULL)
    {
        free(level->mobs);
        free(level);

        return NULL;
    }

    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        level->tiles[y] = malloc(sizeof(Tile) * MAX_WIDTH);

        // check for OOM
        if (level->tiles[y] == NULL)
        {
            free(level->tiles[y]);
            free(level->mobs);
            //free(level); // TODO free rest of tiles FIXME possible memory leak

            return NULL;
        }

        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            level->tiles[y][x] = create_tile(TILE_NONE);
        }
    }

    return level;
}

Tile *get_tile(const Level *level, RL_coords_t coords)
{
    if (coords.y >= MAX_HEIGHT || coords.x >= MAX_WIDTH || coords.y < 0 || coords.x < 0)
        return NULL;

    return &level->tiles[coords.y][coords.x];
}

Mob *get_enemy(const Level *level, RL_coords_t coords)
{
    if (coords.y >= MAX_HEIGHT || coords.x >= MAX_WIDTH || coords.y < 0 || coords.x < 0)
        return NULL;

    for (int i = 0; i < MAX_MOBS; ++i)
    {
        if (level->mobs[i] == NULL)
            continue;
        if (level->mobs[i]->coords.x == coords.x && level->mobs[i]->coords.y == coords.y)
            return level->mobs[i];
    }

    return NULL;
}

Mob *get_mob(const Level *level, RL_coords_t coords)
{
    if (coords.y >= MAX_HEIGHT || coords.x >= MAX_WIDTH || coords.y < 0 || coords.x < 0)
        return NULL;

    if (level->player != NULL)
    {
        if (level->player->coords.x == coords.x && level->player->coords.y == coords.y)
            return level->player;
    }

    return get_enemy(level, coords);
}

int place_on_tile(Mob *mob, int tileType, const Level *level)
{
    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            if (level->tiles[y][x].type == tileType)
            {
                mob->coords.x = x;
                mob->coords.y = y;

                return 1;
            }
        }
    }

    return 0;
}

// TODO handle NULL exception
int move_mob(Mob *mob, RL_coords_t coords, Level *level)
{
    if (mob == NULL)
        return 0;

    // check for mob
    Mob *target = get_mob(level, coords);

    if (target != NULL)
        return 0;

    const Tile *t;
    t = get_tile(level, coords);

    if (t != NULL && is_passable(*t))
    {
        mob->coords.x = coords.x;
        mob->coords.y = coords.y;

        return 1;
    }
    else
        return 0;
}
