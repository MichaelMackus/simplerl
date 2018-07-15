#include "map.h"
#include "random.h"
#include <stdlib.h>
#include <memory.h>

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
    player->hp = 10;
    player->maxHP = 10;
    player->minDamage = 3;
    player->maxDamage = 5;

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

int init_level(Level *level, Mob *player)
{
    if (level == NULL)
        // simple error case
        return 0;

    // do this otherwise initial seed will always be the same
    seed_random();

    // randomly generate map
    randomly_fill_tiles(level);

    // randomly populate *new* levels with max of MAX_MOBS / 2
    randomly_fill_mobs(level, MAX_MOBS / 2);

    return 1;
}

const Tile *get_tile(const Level *level, Coords coords)
{
    if (coords.y >= MAX_HEIGHT || coords.x >= MAX_WIDTH || coords.y < 0 || coords.x < 0)
        return NULL;

    return &level->tiles[coords.y][coords.x];
}

Mob *get_mob(const Level *level, Coords coords)
{
    if (coords.y >= MAX_HEIGHT || coords.x >= MAX_WIDTH || coords.y < 0 || coords.x < 0)
        return NULL;

    for (int i = 0; i < MAX_MOBS; ++i)
    {
        if (level->mobs[i] == NULL)
            continue;
        if (level->mobs[i]->x == coords.x && level->mobs[i]->y == coords.y)
            return level->mobs[i];
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

// TODO handle NULL exception
int move_mob(Mob *mob, Coords coords, Level *level)
{
    if (mob == NULL)
        return 0;

    // check for mob
    Mob *enemy = get_mob(level, coords);

    if (enemy != NULL)
        return 0;

    const Tile *t;
    t = get_tile(level, coords);

    if (t != NULL && is_passable(*t))
    {
        mob->x = coords.x;
        mob->y = coords.y;

        return 1;
    }
    else
        return 0;
}
