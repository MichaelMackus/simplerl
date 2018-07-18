#include "game.h"
#include "message.h"
#include <stdlib.h>
#include <memory.h>

void taint(const Coords playerCoords, Level *level);
int init_level(Level *level, Mob *player)
{
    if (level == NULL)
        // simple error case
        return 0;

    level->player = player;

    // randomly generate map
    randomly_fill_tiles(level);

    // randomly populate *new* levels with max of MAX_MOBS / 2
    randomly_fill_mobs(level, MAX_MOBS / 2);

    // place player on upstair
    place_on_tile(player, TILE_STAIR_UP, level);

    // set smell around player before movement (also done in move_player)
    taint(player->coords, level);

    return 1;
}

int increase_depth(Dungeon *dungeon);
int decrease_depth(Dungeon *dungeon);
void move_player(Mob *player, Coords coords, Level *level);
void run_player(Mob *player, Direction dir, Level *level);
void tick(Dungeon *dungeon);
void tick_mobs(Level *level);
void cleanup(Dungeon *dungeon);
int gameloop(Dungeon *dungeon, char input)
{
    Level *level = dungeon->level;
    Mob *player = dungeon->player;

    if (is_resting(player) || is_running(player))
        input = '.'; // do nothing

    // close inventory menu each turn, TODO inventory management
    dungeon->player->attrs.inMenu = 0;

    switch (input)
    {
        const Tile *t;
        const Mob *m;

        case 'Q':
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

        case 'H':
            run_player(player, direction(-1, 0), level);
            break;
        case 'L':
            run_player(player, direction(1, 0), level);
            break;
        case 'J':
            run_player(player, direction(0, 1), level);
            break;
        case 'K':
            run_player(player, direction(0, -1), level);
            break;

        case ',':
        case 'g':
            ;
            // get all items from floor
            Tile *tile = get_tile(level, player->coords);
            if (tile != NULL)
            {
                Item *item;
                while ((item = take_item(&tile->items)) != NULL)
                    move_item(item, &player->items);
            }

            break;

        case 'i':
            // open inventory menu
            dungeon->player->attrs.inMenu = 1;
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
    cleanup(dungeon);

    // heal player, increase turn count, and decrement smell
    tick(dungeon);

    // be a bit kind & handle mob AI only when *not* changing depth
    if (level->depth == dungeon->level->depth)
        tick_mobs(level);
    else
        // changed depth - get the new level
        level = dungeon->level;

    // check for player death
    if (player->hp <= 0)
        return GAME_DEATH;

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

void run_player(Mob *player, Direction dir, Level *level)
{
    player->attrs.running = dir;
    move_player(player, xy(player->coords.x + dir.xdir, player->coords.y + dir.ydir), level);
}

// mob AI & spawning
// TODO add simple mob movement (instead of just sitting there)
// TODO add simple sound AI (i.e. mob should be able to hear combat further than they can smell, and also remember that)
void tick_mob(Mob *mob, Level *level);
Coords smelliest(Coords coords, Level *level);
int can_smell(Coords coords, Level *level);
void tick_mobs(Level *level)
{
    for (int i = 0; i < MAX_MOBS; ++i)
        if (level->mobs[i] != NULL)
            tick_mob(level->mobs[i], level);

    // 1/10 chance of new mob every turn
    if (generate(1, 10) == 10)
    {
        // get random coordinates for new mob, must not be near player
        Coords coords = random_passable_coords(level);
        while (can_see(coords, level->player->coords, level->tiles) ||
                can_smell(coords, level))
            coords = random_passable_coords(level);

        Mob *mob = create_mob(level->depth, coords);

        if (mob == NULL)
            return;

        insert_mob(mob, level->mobs);
    }
}

void tick_mob(Mob *mob, Level *level)
{
    Mob *player = level->player;

    if (can_see(mob->coords, player->coords, level->tiles))
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
        if (can_smell(mob->coords, level))
        {
            Coords coords = smelliest(mob->coords, level);
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
    if (player->attrs.exp / 1000 > (player->attrs.level - 1))
    {
        ++player->attrs.level;
        // TODO figure out better stat system
        player->maxHP += 5;
        player->hp = player->maxHP;
    }
}

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
        // heal player every 10 turns
        if (player->hp < player->maxHP)
            player->hp += 1;
    }

    ++dungeon->turn;
}

// cleanup dead mobs
void cleanup(Dungeon *dungeon)
{
    Level *level = dungeon->level;
    Mob *player = level->player;

    for (int i = 0; i < MAX_MOBS; ++i)
    {
        if (level->mobs[i] != NULL)
        {
            Mob *mob = level->mobs[i];
            if (mob->hp <= 0)
            {
                // transfer items to floor
                Tile *t = get_tile(level, mob->coords);
                if (t != NULL) move_items(mob->items, &t->items);
                else free_items(mob->items);

                // add to killed mobs
                kill_mob(mob, &dungeon->killed);

                // clear mob in level & reward exp
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
// TODO should probably remove smells when changing depth
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
// TODO should probably remove smells when changing depth
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
    Level *level = dungeon->level;

    // update seen tiles (done here to update before first turn)
    // TODO make more efficient
    if (!is_resting(player))
        for (int y = 0; y < MAX_HEIGHT; ++y)
            for (int x = 0; x < MAX_WIDTH; ++x)
                if (can_see(player->coords, xy(x, y), level->tiles))
                    level->tiles[y][x].seen = 1;

    if (is_resting(player))
    {
        if (player->hp >= player->maxHP)
            dungeon->player->attrs.resting = 0;

        // if player can see any mobs, reset resting flag
        for (int i = 0; i < MAX_MOBS; ++i)
            if (level->mobs[i] != NULL &&
                can_see(player->coords, level->mobs[i]->coords, level->tiles))
                    dungeon->player->attrs.resting = 0;

        // if we're still resting, don't handle input
        if (is_resting(player))
            return 0;
    }

    if (is_running(player))
    {
        Direction dir = player->attrs.running;
        Coords target = xy(player->coords.x + dir.xdir, player->coords.y + dir.ydir);
        Tile *tile = get_tile(level, target);

        // if player can see any mobs, reset running flag
        for (int i = 0; i < MAX_MOBS; ++i)
            if (level->mobs[i] != NULL &&
                    can_see(player->coords, level->mobs[i]->coords, level->tiles))
                dungeon->player->attrs.running = direction(0, 0);

        if (tile == NULL || !is_passable(*tile) ||
                get_enemy(level, target) != NULL)
            player->attrs.running = direction(0, 0);

        // if we're still running, move the player and don't handle input
        if (is_running(player))
        {
            move_player(player, target, level);

            return 0;
        }
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

int can_smell(Coords coords, Level *level)
{
    Tile *tile;

    tile = get_tile(level, xy(coords.x + 1, coords.y));
    if (tile != NULL && is_passable(*tile) && tile->smell > 0)
        return 1;

    tile = get_tile(level, xy(coords.x - 1, coords.y));
    if (tile != NULL && is_passable(*tile) && tile->smell > 0)
        return 1;

    tile = get_tile(level, xy(coords.x, coords.y + 1));
    if (tile != NULL && is_passable(*tile) && tile->smell > 0)
        return 1;

    tile = get_tile(level, xy(coords.x, coords.y - 1));
    if (tile != NULL && is_passable(*tile) && tile->smell > 0)
        return 1;

    return 0;
}
