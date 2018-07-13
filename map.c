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

    return level;
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
