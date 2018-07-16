#include "game.h"
#include "message.h"
#include "random.h"
#include <ncurses.h>
#include <stdlib.h>
#include <memory.h>

int init_level(Level *level, Mob *player)
{
    if (level == NULL)
        // simple error case
        return 0;

    level->player = player;

    // do this otherwise initial seed will always be the same
    seed_random();

    // randomly generate map
    randomly_fill_tiles(level);

    // randomly populate *new* levels with max of MAX_MOBS / 2
    randomly_fill_mobs(level, MAX_MOBS / 2, true);

    return 1;
}

int increase_depth(Dungeon *dungeon);
int decrease_depth(Dungeon *dungeon);
void move_player(Mob *player, Coords coords, Level *level);
void tick(Dungeon *dungeon);
void tick_mob(Mob *mob, Level *level);
int gameloop(Dungeon *dungeon)
{
    Level *level = dungeon->level;
    Mob *player = dungeon->player;

    // Set smell for initial tile so that the smell for the current tile is
    // always set, in case the player doesn't move or this is the first
    // iteration.
    level->tiles[player->coords.y][player->coords.x].smell = INITIAL_SMELL;

    // cleanup dead mobs, heal player, spawn new mobs
    tick(dungeon);

    // handle input
    char ch = getch();
    switch (ch)
    {
        const Tile *t;
        const Mob *m;

        case 'q':
            return GAME_QUIT;

        case 'h':
            move_player(player, xy(player->coords.x - 1, player->coords.y), level);
            break;
        case 'l':
            move_player(player, xy(player->coords.x + 1, player->coords.y), level);
            break;
        case 'j':
            move_player(player, xy(player->coords.x, player->coords.y + 1), level);
            break;
        case 'k':
            move_player(player, xy(player->coords.x, player->coords.y - 1), level);
            break;

        case '>': // check for downstair
            t = get_tile(level, player->coords);

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

            t = get_tile(level, player->coords);

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

    // check for player death (i.e. damaged self)
    if (player->hp <= 0)
        return GAME_DEATH;

    // mob AI
    for (int i = 0; i < MAX_MOBS; ++i)
        if (level->mobs[i] != NULL)
            tick_mob(level->mobs[i], level);

    // check for player death
    if (player->hp <= 0)
        return GAME_DEATH;

    // update seen tiles
    for (int y = 0; y < MAX_HEIGHT; ++y)
        for (int x = 0; x < MAX_WIDTH; ++x)
            if (can_see(player, xy(x, y), level->tiles))
                level->tiles[y][x].seen = 1;

    return GAME_PLAYING;
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

// returns -1 on move, 0 or more damage on attack
int move_or_attack(Mob *attacker, Coords coords, Level *level)
{
    // first, check for mob
    Mob *target = get_mob(level, coords);

    if (target != NULL)
        return attack(attacker, target);
    else
        move_mob(attacker, coords, level);

    return -1;
}

void move_player(Mob *player, Coords coords, Level *level)
{
    int dmg = move_or_attack(player, coords, level);

    // there was an enemy there!
    if (dmg > 0)
        message("You hit it for %d damage!", dmg); // TODO mob name
    else if (dmg == 0)
        message("You missed!");

    // set smell for tile
    level->tiles[player->coords.y][player->coords.x].smell = INITIAL_SMELL;
}

void tick_mob(Mob *mob, Level *level)
{
    Mob *player = level->player;

    if (can_see(mob, player->coords, level->tiles))
    {
        int dmg = -1;

        // advance mob towards player, if there is no enemy at target spot
        if (player->coords.x > mob->coords.x &&
                is_passable(level->tiles[mob->coords.y][mob->coords.x + 1]) &&
                get_enemy(level, xy(mob->coords.x + 1, mob->coords.y)) == NULL)
            dmg = move_or_attack(mob, xy(mob->coords.x + 1, mob->coords.y), level);
        else if (player->coords.x < mob->coords.x &&
                is_passable(level->tiles[mob->coords.y][mob->coords.x - 1]) &&
                get_enemy(level, xy(mob->coords.x - 1, mob->coords.y)) == NULL)
            dmg = move_or_attack(mob, xy(mob->coords.x - 1, mob->coords.y), level);
        else if (player->coords.y > mob->coords.y &&
                is_passable(level->tiles[mob->coords.y + 1][mob->coords.x]) &&
                get_enemy(level, xy(mob->coords.x, mob->coords.y + 1)) == NULL)
            dmg = move_or_attack(mob, xy(mob->coords.x, mob->coords.y + 1), level);
        else if (player->coords.y < mob->coords.y &&
                is_passable(level->tiles[mob->coords.y - 1][mob->coords.x]) &&
                get_enemy(level, xy(mob->coords.x, mob->coords.y - 1)) == NULL)
            dmg = move_or_attack(mob, xy(mob->coords.x, mob->coords.y - 1), level);
        else
            return;

        if (dmg > 0)
            message("You got hit for %d damage!", dmg); // TODO mob name
        else if (dmg == 0)
            message("It missed!");
    }

    // TODO implement smell
}

void tick(Dungeon *dungeon)
{
    Level *level = dungeon->level;
    Mob *player = level->player;

    // spawn new mob every 10 turns
    if (dungeon->turn % 10 == 0)
        randomly_fill_mobs(level, 1, false);

    // cleanup mobs
    for (int i = 0; i < MAX_MOBS; ++i)
    {
        if (level->mobs[i] != NULL)
        {
            Mob *mob = level->mobs[i];
            if (mob->hp <= 0)
            {
                free(mob);
                // TODO transfer items to floor
                level->mobs[i] = NULL;
            }
        }
    }

    // heal player every 10 turns
    if (player->hp < player->maxHP && dungeon->turn % 10 == 0)
        player->hp += 1;

    ++dungeon->turn;
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
