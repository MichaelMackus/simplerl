#include "game.h"
#include "message.h"
#include "random.h"
#include <ncurses.h>
#include <stdlib.h>
#include <memory.h>

void taint(const Coords playerCoords, Level *level);
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

    // place player on upstair
    place_on_tile(player, TILE_STAIR_UP, level);

    // set smell around player before movement (also done in move_player)
    taint(player->coords, level);

    return 1;
}

int increase_depth(Dungeon *dungeon);
int decrease_depth(Dungeon *dungeon);
void move_player(Mob *player, Coords coords, Level *level);
void tick(Dungeon *dungeon);
void tick_mob(Mob *mob, Level *level);
int handle_input(Dungeon *dungeon);
void cleanup(Level *level);
int gameloop(Dungeon *dungeon)
{
    Level *level = dungeon->level;
    Mob *player = dungeon->player;

    char ch = '.'; // do nothing
    // handle input
    if (handle_input(dungeon))
        ch = getch();

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

        case 'R':
            dungeon->player->attrs.resting = 1;
            break;

        case '>': // check for downstair
            t = get_tile(level, player->coords);

            if (t == NULL)
                return GAME_ERROR;
            if (t->type == TILE_STAIR_DOWN)
                if (dungeon->level->depth == MAX_LEVEL)
                    return GAME_WON;
                if (!increase_depth(dungeon))
                    return GAME_OOM;

            break;

        case '<': // check for upstair

            t = get_tile(level, player->coords);

            if (t == NULL)
                return GAME_ERROR;
            if (t->type == TILE_STAIR_UP)
                if (dungeon->level->depth == 1)
                    return GAME_QUIT;
                if (!decrease_depth(dungeon))
                    return GAME_OOM;

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

    // cleanup dead mobs
    cleanup(level);

    // heal player, spawn new mobs
    // TODO move this before loop to prevent mobs jumping player?
    tick(dungeon);

    // check for player death
    if (player->hp <= 0)
        return GAME_DEATH;

    // update seen tiles
    // TODO make more efficient
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
    else
        // movement - set the smell of the player again
        taint(player->coords, level);
}

Coords smelliest(Coords coords, Level *level);
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
    else
    {
        // if mob can't see, they can still smell the player (thanks NetHack!)
        Coords coords = smelliest(mob->coords, level);
        const Tile *tile = get_tile(level, coords);
        if (tile != NULL && is_passable(*tile) &&
                !(coords.x == mob->coords.x && coords.y == mob->coords.y))
        {
            move_or_attack(mob, coords, level);
        }
    }
}

void reward_exp(Mob *player, Mob *mob)
{
    // calculate exp based on difficulty of mob
    player->attrs.exp += 100*mob->difficulty;

    // max level
    if (player->attrs.level == MAX_PLAYER_LEVEL)
        return;

    // level up condition
    // TODO this is getting triggered on 0 exp sometimes
    if (player->attrs.exp % 1000 == 0)
    {
        ++player->attrs.level;
        // TODO figure out better stat system
        ++player->minDamage;
        ++player->maxDamage;
        player->maxHP += 5;
        player->hp = player->maxHP;
    }
}

// TODO add simple mob movement (instead of just sitting there)
// TODO add simple sound AI (i.e. mob should be able to hear combat further than they can smell, and also remember that)
void tick(Dungeon *dungeon)
{
    Level *level = dungeon->level;
    Mob *player = level->player;

    // reduce smell on all tiles not on current player x, y
    for (int y = 0; y < MAX_HEIGHT; ++y)
        for (int x = 0; x < MAX_WIDTH; ++x)
            if (!(player->coords.x == x && player->coords.y == y) &&
                    level->tiles[y][x].smell > 0)
                --level->tiles[y][x].smell;

    if (dungeon->turn % 10 == 0)
    {
        // spawn new mob every 10 turns
        randomly_fill_mobs(level, 1, false);

        // heal player every 10 turns
        if (player->hp < player->maxHP)
            player->hp += 1;
    }

    // mob AI
    for (int i = 0; i < MAX_MOBS; ++i)
        if (level->mobs[i] != NULL)
            tick_mob(level->mobs[i], level);

    ++dungeon->turn;
}

// cleanup dead mobs
void cleanup(Level *level)
{
    Mob *player = level->player;

    for (int i = 0; i < MAX_MOBS; ++i)
    {
        if (level->mobs[i] != NULL)
        {
            Mob *mob = level->mobs[i];
            if (mob->hp <= 0)
            {
                free(mob);
                // TODO transfer items to floor
                reward_exp(player, mob);
                level->mobs[i] = NULL;
            }
        }
    }
}

// taint smell aura around player
void taint(const Coords playerCoords, Level *level)
{
    // NOTE: all these tiles *except* the current player tile will get
    // decremented by 1 in gameloop after this code is run
    int coords[13][3] = {
        { playerCoords.x, playerCoords.y, INITIAL_SMELL },
        { playerCoords.x - 1, playerCoords.y, INITIAL_SMELL - 1 },
        { playerCoords.x - 1, playerCoords.y - 1, INITIAL_SMELL - 1 },
        { playerCoords.x - 1, playerCoords.y + 1, INITIAL_SMELL - 1 },
        { playerCoords.x - 2, playerCoords.y, INITIAL_SMELL - 2 },
        { playerCoords.x, playerCoords.y - 1, INITIAL_SMELL - 1 },
        { playerCoords.x, playerCoords.y - 2, INITIAL_SMELL - 2 },
        { playerCoords.x + 1, playerCoords.y, INITIAL_SMELL - 1 },
        { playerCoords.x + 1, playerCoords.y - 1, INITIAL_SMELL - 1 },
        { playerCoords.x + 1, playerCoords.y + 1, INITIAL_SMELL - 1 },
        { playerCoords.x + 2, playerCoords.y, INITIAL_SMELL - 2 },
        { playerCoords.x, playerCoords.y + 1, INITIAL_SMELL - 1 },
        { playerCoords.x, playerCoords.y + 2, INITIAL_SMELL - 2 }
    };

    for (int i = 0; i < 13; ++i)
    {
        Coords location = xy(coords[i][0], coords[i][1]);
        int smell = coords[i][2];

        // set smell if tile & greater than current smell
        Tile *tile = get_tile(level, location);
        if (tile != NULL && tile->smell < smell)
            tile->smell = smell;
    }
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

int handle_input(Dungeon *dungeon)
{
    Mob *player = dungeon->player;

    if (player->hp >= player->maxHP)
        dungeon->player->attrs.resting = 0;

    if (dungeon->player->attrs.resting)
    {
        // if player can see any mobs, reset resting flag
        Level *level = dungeon->level;
        for (int i = 0; i < MAX_MOBS; ++i)
            if (level->mobs[i] != NULL)
            {
                Mob *mob = level->mobs[i];
                if (can_see(player, mob->coords, level->tiles))
                {
                    dungeon->player->attrs.resting = 0;

                    return 1;
                }
            }

        return 0;
    }

    return 1;
}

// TODO allow mobs to smell diagonally
Coords smelliest(Coords coords, Level *level)
{
    Tile **tiles = level->tiles;
    Coords smelliest = coords;
    int smell = 0;

    if (get_tile(level, xy(coords.x, coords.y + 1)) != NULL &&
            get_tile(level, xy(coords.x, coords.y + 1))->smell > smell &&
            is_passable(*get_tile(level, xy(coords.x, coords.y + 1))) &&
            get_enemy(level, xy(coords.x, coords.y + 1)) == NULL)
    {
        smelliest.y = coords.y + 1;
        smelliest.x = coords.x;
        smell = get_tile(level, xy(coords.x, coords.y + 1))->smell;
    }

    if (get_tile(level, xy(coords.x, coords.y - 1)) != NULL &&
            get_tile(level, xy(coords.x, coords.y - 1))->smell > smell &&
            is_passable(*get_tile(level, xy(coords.x, coords.y - 1))) &&
            get_enemy(level, xy(coords.x, coords.y - 1)) == NULL)
    {
        smelliest.y = coords.y - 1;
        smelliest.x = coords.x;
        smell = get_tile(level, xy(coords.x, coords.y - 1))->smell;
    }

    if (get_tile(level, xy(coords.x + 1, coords.y)) != NULL &&
            get_tile(level, xy(coords.x + 1, coords.y))->smell > smell &&
            is_passable(*get_tile(level, xy(coords.x + 1, coords.y))) &&
            get_enemy(level, xy(coords.x + 1, coords.y)) == NULL)
    {
        smelliest.y = coords.y;
        smelliest.x = coords.x + 1;
        smell = get_tile(level, xy(coords.x + 1, coords.y))->smell;
    }

    if (get_tile(level, xy(coords.x - 1, coords.y)) != NULL &&
            get_tile(level, xy(coords.x - 1, coords.y))->smell > smell &&
            is_passable(*get_tile(level, xy(coords.x - 1, coords.y))) &&
            get_enemy(level, xy(coords.x - 1, coords.y)) == NULL)
    {
        smelliest.y = coords.y;
        smelliest.x = coords.x - 1;
        smell = get_tile(level, xy(coords.x - 1, coords.y))->smell;
    }

    return smelliest;
}
