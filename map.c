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
    player->x = 40;
    player->y = 15;
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

    return level;
}

void increase_depth(Dungeon dungeon)
{
}

void decrease_depth(Dungeon dungeon)
{
}
