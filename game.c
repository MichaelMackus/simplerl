#include "game.h"
#include "message.h"
#include <ncurses.h>
#include <stdlib.h>
#include <memory.h>

int increase_depth(Dungeon *dungeon);
int decrease_depth(Dungeon *dungeon);
void move_or_attack(Mob *player, Coords coords, Level *level);
int gameloop(Dungeon *dungeon)
{
    Level *level = dungeon->level;
    Mob *player = dungeon->player;

    // Set smell for initial tile so that the smell for the current tile is
    // always set, in case the player doesn't move or this is the first
    // iteration.
    level->tiles[player->y][player->x].smell = INITIAL_SMELL;

    // handle input
    char ch = getch();
    switch (ch)
    {
        const Tile *t;
        const Mob *m;

        case 'q':
            return GAME_QUIT;

        case 'h':
            move_or_attack(player, xy(player->x - 1, player->y), level);
            break;
        case 'l':
            move_or_attack(player, xy(player->x + 1, player->y), level);
            break;
        case 'j':
            move_or_attack(player, xy(player->x, player->y + 1), level);
            break;
        case 'k':
            move_or_attack(player, xy(player->x, player->y - 1), level);
            break;

        case '>': // check for downstair
            t = get_tile(level, xy(player->x, player->y));

            if (t == NULL)
                return GAME_ERROR;
            if (t->type == TILE_STAIR_DOWN)
            {
                if (dungeon->level->depth == MAX_LEVEL)
                    return GAME_WON;
                if (!increase_depth(dungeon))
                    return GAME_OOM;

                message("Current level: %d", dungeon->level->depth);
            }

            break;

        case '<': // check for upstair

            t = get_tile(level, xy(player->x, player->y));

            if (t == NULL)
                return GAME_ERROR;
            if (t->type == TILE_STAIR_UP)
            {
                if (dungeon->level->depth == 1)
                    return GAME_QUIT;
                if (!decrease_depth(dungeon))
                    return GAME_OOM;

                message("Current level: %d", dungeon->level->depth);
            }

            break;

        case 'D':
            return GAME_DEATH;
        case 'W':
            return GAME_WON;
        case 'E':
            return GAME_OOM;
    }

    // cleanup dead mobs
    for (int i = 0; i < MAX_MOBS; ++i)
    {
        if (dungeon->level->mobs[i] != NULL)
        {
            if (dungeon->level->mobs[i]->hp <= 0)
            {
                Mob *mob = dungeon->level->mobs[i];
                free(mob);
                // TODO transfer items to floor
                dungeon->level->mobs[i] = NULL;
            }
        }
    }

    // TODO simple AI

    // update seen tiles
    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            if (can_see(player, xy(x, y), level->tiles))
                level->tiles[y][x].seen = 1;
        }
    }

    return GAME_PLAYING;
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

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

void move_or_attack(Mob *player, Coords coords, Level *level)
{
    // first, check for mob
    Mob *enemy = get_mob(level, coords);

    if (enemy != NULL)
    {
        // attack mob & insert damage message
        int dmg = attack(player, enemy);
        if (dmg > 0)
            message("You hit it for %d damage!", dmg); // TODO mob name
        else
            message("You missed!");
    }
    else
    {
        move_mob(player, coords, level);

        // set smell for tile
        level->tiles[player->y][player->x].smell = INITIAL_SMELL;
    }
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
