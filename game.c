#include "game.h"
#include "message.h"
#include "random.h"
#include <ncurses.h>
#include <stdlib.h>
#include <memory.h>

int alert_depth(Level *level, const char **messages);

int init_level(Level *level, const char **messages)
{
    if (level == NULL || messages == NULL)
    {
        // simple error case
        return 1;
    }

    // do this otherwise initial seed will always be the same
    seed_random();

    // TODO randomly populate *new* levels with:
    //          - tiles
    //          - mobs
    //          - stairs

    // randomly generate map
    randomly_fill_tiles(level);

    // alert level depth on game start
    alert_depth(level, messages);

    return 0;
}

int move_mob(Mob *mob, int y, int x, Level *level);
void attack(Mob *attacker, Mob *target);
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
                attack(player, get_mob(level, player->x - 1, player->y));

            break;
        case 'l':
            if (!move_mob(player, player->y, player->x + 1, level))
                attack(player, get_mob(level, player->x + 1, player->y));

            break;
        case 'j':
            if (!move_mob(player, player->y + 1, player->x, level))
                attack(player, get_mob(level, player->x, player->y + 1));

            break;
        case 'k':
            if (!move_mob(player, player->y - 1, player->x, level))
                attack(player, get_mob(level, player->x, player->y - 1));

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

                alert_depth(dungeon->level, messages);
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

                alert_depth(dungeon->level, messages);
            }

            break;

        case 'D':
            return GAME_DEATH;
        case 'W':
            return GAME_WON;
        case 'E':
            return GAME_OOM;
    }

    // TODO cleanup dead mobs, potentially transferring items to square

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

    if (mob != NULL && t != NULL && is_passable(*t))
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
    // for now, just free & remove the target
    // TODO want to reduce health to 0
}

// insert the current dungeon depth to messages array
int alert_depth(Level *level, const char **messages)
{
    // allocate memory for message
    const char *message = create_message("Current level: %d", level->depth);
    if (message == NULL)
        return 1;

    // insert the message
    insert_message(message, messages);

    return 0;
}
