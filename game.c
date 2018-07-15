#include "game.h"
#include "message.h"
#include <ncurses.h>
#include <stdlib.h>
#include <memory.h>

int move_mob(Mob *mob, int y, int x, Level *level);
void attack(Mob *attacker, Mob *target);
int increase_depth(Dungeon *dungeon);
int decrease_depth(Dungeon *dungeon);
int gameloop(Dungeon *dungeon, const char **messages)
{
    Level *level = dungeon->level;
    Mob *player = dungeon->player;

    // handle input
    char ch = getch();
    switch (ch)
    {
        const Tile *t;
        const Mob *m;

        case 'q':
            return GAME_QUIT;
        case 'h':
            if (!move_mob(player, player->y, player->x - 1,level))
                attack(player, get_mob(level, player->y, player->x - 1));

            break;
        case 'l':
            if (!move_mob(player, player->y, player->x + 1, level))
                attack(player, get_mob(level, player->y, player->x + 1));

            break;
        case 'j':
            if (!move_mob(player, player->y + 1, player->x, level))
                attack(player, get_mob(level, player->y + 1, player->x));

            break;
        case 'k':
            if (!move_mob(player, player->y - 1, player->x, level))
                attack(player, get_mob(level, player->y - 1, player->x));

            break;

        case '>': // check for downstair
            t = get_tile(level, player->y, player->x);

            if (t == NULL)
                return GAME_ERROR;
            if (t->type == TILE_STAIR_DOWN)
            {
                if (dungeon->level->depth == MAX_LEVEL)
                    return GAME_WON;
                if (!increase_depth(dungeon))
                    return GAME_OOM;

                insert_message(create_message("Current level: %d", dungeon->level->depth), messages);
            }

            break;

        case '<': // check for upstair

            t = get_tile(level, player->y, player->x);

            if (t == NULL)
                return GAME_ERROR;
            if (t->type == TILE_STAIR_UP)
            {
                if (dungeon->level->depth == 1)
                    return GAME_QUIT;
                if (!decrease_depth(dungeon))
                    return GAME_OOM;

                insert_message(create_message("Current level: %d", dungeon->level->depth), messages);
            }

            break;

        case 'D':
            return GAME_DEATH;
        case 'W':
            return GAME_WON;
        case 'E':
            return GAME_OOM;
    }

    // TODO possibly do cleanup of dead mobs here?

    return GAME_PLAYING;
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

// move a mob to x, y
// return 1 on success, or 0 if impassable
// TODO handle NULL exception
int move_mob(Mob *mob, int y, int x, Level *level)
{
    const Tile *t;
    t = get_tile(level, y, x);

    // check for mob
    Mob *enemy = get_mob(level, y, x);

    if (enemy == NULL && mob != NULL && t != NULL && is_passable(*t))
    {
        mob->x = x;
        mob->y = y;

        return 1;
    }
    else
        return 0;
}

// try to attack x, y
// if no mob found at x, y do nothing
void attack(Mob *attacker, Mob *target)
{
    if (attacker == NULL || target == NULL)
        return;

    // TODO basic RNG, attack, health, etc.
    // for now, just reduce health to 0
    target->hp = 0;
}

// change current depth to next level deep
// if there is no next level, create one
// return 0 on error
int increase_depth(Dungeon *dungeon)
{
    if (dungeon->level->depth == MAX_LEVEL)
        return 0;

    if (dungeon->level->next == NULL)
    {
        // initialize next level
        Level *level;
        level = create_level(dungeon->level->depth + 1);

        // set our link relationship to next level
        dungeon->level->next = level;
        level->prev = dungeon->level;

        // randomly fill dungeon
        if (!init_level(level, dungeon->player))
            return 0;
    }

    // now we can update the current level
    dungeon->level = dungeon->level->next;

    // place player on upstair
    place_on_tile(dungeon->player, TILE_STAIR_UP, dungeon->level);

    return 1;
}


// change current depth to previous level
// return 0 on error
int decrease_depth(Dungeon *dungeon)
{
    if (dungeon->level->prev == NULL)
        return 0;

    // set level to previous level
    dungeon->level = dungeon->level->prev;

    // place player on downstair
    place_on_tile(dungeon->player, TILE_STAIR_DOWN, dungeon->level);

    return 1;
}
