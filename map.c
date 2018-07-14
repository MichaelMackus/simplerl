#include "map.h"
#include <stdlib.h>

Dungeon *create_dungeon()
{
    // allocate dungeon
    Dungeon *dungeon;
    dungeon = malloc(sizeof(Dungeon));

    // handle out of memory case
    if (dungeon == NULL)
        return NULL;

    // allocate player
    Mob *player;
    player = malloc(sizeof(Mob));
    dungeon->player = player;

    // handle out of memory case
    if (player == NULL)
    {
        free(dungeon);

        return NULL;
    }

    // initialize player
    player->type = MOB_PLAYER;
    player->symbol = '@';

    // initialize first level
    Level *level = create_level(1);
    dungeon->level = level;

    // handle out of memory case
    if (level == NULL)
    {
        free(player);
        free(dungeon);

        return NULL;
    }

    return dungeon;
}

Level *create_level(int depth)
{
    Level *level;
    level = malloc(sizeof(Level));

    // check for OOM
    if (level == NULL)
        return NULL;

    // initialize mobs array
    level->mobs = malloc(sizeof(Mob) * MAX_MOBS);

    // check for OOM
    if (level->mobs == NULL)
    {
        free(level);

        return NULL;
    }

    // initialize depth
    level->depth = depth;

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
            Tile t;
            t.type = TILE_NONE;
            level->tiles[y][x] = t;
        }
    }

    return level;
}

const Tile *get_tile(const Level *level, unsigned int y, unsigned int x)
{
    if (y >= MAX_HEIGHT || x >= MAX_WIDTH)
        return NULL;

    return &level->tiles[y][x];
}

Mob *get_mob(const Level *level, unsigned int y, unsigned int x)
{
    if (y >= MAX_HEIGHT || x >= MAX_WIDTH)
        return NULL;

    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            for (int i = 0; i < MAX_MOBS; ++i)
            {
                if (level->mobs[i].x == x && level->mobs[i].y == y)
                    return &level->mobs[i];
            }
        }
    }

    return NULL;
}

int place_on_tile(Mob *mob, int tileType, const Level *level)
{
    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            if (level->tiles[y][x].type == tileType)
            {
                mob->x = x;
                mob->y = y;

                return 1;
            }
        }
    }

    return 0;
}
