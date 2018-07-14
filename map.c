#include "map.h"
#include <stdlib.h>

Dungeon *create_dungeon()
{
    // allocate dungeon
    Dungeon *dungeon;
    dungeon = malloc(sizeof(Dungeon));

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

    // initialize mobs array
    level->mobs = malloc(sizeof(Mob) * MAX_MOBS);

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
    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        level->tiles[y] = malloc(sizeof(Tile) * MAX_WIDTH);
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            Tile t;
            if (y == 0)
                t.type = TILE_WALL;
            else if (x == 0)
                t.type = TILE_WALL_SIDE;
            else if (x == MAX_WIDTH-1)
                t.type = TILE_WALL_SIDE;
            else if (y == MAX_HEIGHT-1)
                t.type = TILE_WALL;
            else
                t.type = TILE_FLOOR;
            level->tiles[y][x] = t;
        }
    }

    return level;
}

const Tile *get_tile(Level *level, int y, int x)
{
    if (y >= MAX_HEIGHT || x >= MAX_WIDTH)
        return NULL;

    return &level->tiles[y][x];
}

Mob *get_mob(Level *level, int y, int x)
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

int increase_depth(Dungeon *dungeon)
{
    if (dungeon->level->depth == MAX_LEVEL)
        return 0;

    // initialize next level
    Level *level;
    level = create_level(dungeon->level->depth + 1);

    // set our link relationship to next level
    dungeon->level->next = level;
    level->prev = dungeon->level;

    // now we can update the current level
    dungeon->level = level;

    return 1;
}

int decrease_depth(Dungeon *dungeon)
{
    if (dungeon->level->prev == NULL)
        return 0;

    // set level to previous level
    dungeon->level = dungeon->level->prev;

    return 1;
}
