#include "dungeon.h"
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <time.h>

Dungeon *create_dungeon()
{
    // do this otherwise initial seed will always be the same
    init_random(time(0));

    // allocate dungeon
    Dungeon *dungeon;
    dungeon = malloc(sizeof(Dungeon));

    // handle out of memory case
    if (dungeon == NULL)
        return NULL;

    dungeon->turn = 0;
    dungeon->killed = NULL;

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
    player->minDamage = 1;
    player->maxDamage = 3;
    player->equipment = (Equipment) {0};
    player->attrs = (PlayerAttributes) {0};
    player->attrs.expNext = 1000;
    player->attrs.level = 1;
    player->itemCount = 0;
    memset(player->items, 0, MAX_INVENTORY_ITEMS);

    // give player some simple equipment
    Item *gold = create_item(1, ITEM_GOLD);
    Item *armor = leather();
    Item *weapon = quarterstaff();
    give_mob_item(player, gold);
    if (give_mob_item(player, armor))
        player->equipment.armor = armor;
    if (give_mob_item(player, weapon))
        player->equipment.weapon = weapon;

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

int max_depth(Dungeon *dungeon)
{
    Level *cur = dungeon->level;
    while (cur->next)
        cur = cur->next;

    return cur->depth;
}

void randomly_fill_tiles(Level *level);
void randomly_fill_mobs(Level *level, int max);
int init_level(Level *level, Mob *player)
{
    if (level == NULL)
        // simple error case
        return 0;

    level->player = player;

    // randomly generate map
    randomly_fill_tiles(level);

    // place player on upstair
    player->coords = level->upstair_loc;

    // randomly populate *new* levels with max of MAX_MOBS / 2
    randomly_fill_mobs(level, MAX_MOBS / 2);

    // set smell around player before movement (also done in move_player)
    taint(player->coords, level);

    return 1;
}

// taint smell aura around player
void taint(const RL_Point playerCoords, Level *level)
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
        RL_Point location = RL_XY(coords[i][0], coords[i][1]);
        int smell = coords[i][2];

        // out of bounds, skip coord
        if (!rl_map_in_bounds(level->map, location)) continue;

        // set smell if tile & greater than current smell
        if (level->smell[(int)location.y][(int)location.x] < smell)
            level->smell[(int)location.y][(int)location.x] = smell;
    }
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

// dungeon generation stuff

void randomly_fill_tiles(Level *level)
{
    if (level == NULL) return;
    level->map = rl_map_create(MAX_WIDTH, MAX_HEIGHT);
    assert(level->map);
    RL_BSP *bsp = rl_mapgen_bsp(level->map, (RL_MapgenConfigBSP) { 3, 5, 3, 5, 1, 1, 1, 1 });

    // randomly place upstairs
    RL_Point up;
    int i = 0;
    while (i < MAX_RANDOM_RECURSION) {
        up = random_passable_coords(level);
        if (rl_map_tile_is(level->map, up, RL_TileRoom)) {
            level->upstair_loc = up;
            break;
        }
    }
    assert(i < MAX_RANDOM_RECURSION);
    assert(rl_map_tile_is(level->map, level->upstair_loc, RL_TileRoom));

    // randomly place downstairs
    // TODO place downstairs at greater distance from upstairs
    // TODO once win condition is defined, don't place downstairs on last level
    RL_Point down;
    i = 0;
    while (i < MAX_RANDOM_RECURSION) {
        down = random_passable_coords(level);
        if (rl_map_tile_is(level->map, down, RL_TileRoom) &&
                !(level->upstair_loc.x == down.x && level->upstair_loc.y == down.y)) {
            level->downstair_loc = down;
            break;
        }
    }
    assert(i < MAX_RANDOM_RECURSION);
    assert(rl_map_tile_is(level->map, level->downstair_loc, RL_TileRoom));

    rl_bsp_destroy(bsp);
}

void randomly_fill_mobs(Level *level, int max)
{
    Mob **mobs = level->mobs;

    int mobIndex = 0;
    int amount = generate(0, max);
    for (int i = 0; i < amount; ++i)
    {
        RL_Point coords = random_passable_coords(level);
        Mob *mob = create_mob(level->depth, coords);

        if (mob == NULL)
            return;

        // don't spawn mobs on stairs
        while ((level->upstair_loc.x == coords.x && level->upstair_loc.y == coords.y) ||
                (level->downstair_loc.x == coords.x && level->downstair_loc.y == coords.y))
        {
            coords = random_passable_coords(level);
        }

        insert_mob(mob, mobs);
    }
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

RL_Point random_coords(Level *level)
{
    RL_Point coords;
    coords.x = generate(0, MAX_WIDTH - 1);
    coords.y = generate(0, MAX_HEIGHT - 1);

    return coords;
}

// FIXME these two functions are a quick fix
RL_Point empty_coords()
{
    RL_Point coords;
    coords.x = MAX_WIDTH;
    coords.y = MAX_HEIGHT;

    return coords;
}

Mob *get_mob(const Level *level, RL_Point coords);
RL_Point random_passable_coords(Level *level)
{
    // do simple brute force attempt to get a passable coord
    int i = 0;
    while (i < MAX_RANDOM_RECURSION)
    {
        RL_Point coords = random_coords(level);
        Mob *m = get_mob(level, coords);
        if (rl_map_is_passable(level->map, coords) && m == NULL)
            return coords;
        ++i;
    }
    assert(i < MAX_RANDOM_RECURSION);

    return empty_coords();
}

Level *create_level(int depth)
{
    Level *level;
    level = malloc(sizeof(Level));

    // check for OOM
    if (level == NULL)
        return NULL;

    // initialize mobs
    for (int i = 0; i < MAX_MOBS; ++i) level->mobs[i] = NULL;

    // initialize depth
    level->depth = depth;

    // initialize vars to null
    level->next = NULL;
    level->prev = NULL;

    // initialize smells & items
    for (int y=0; y<MAX_HEIGHT; ++y) {
        for (int x=0; x<MAX_WIDTH; ++x) {
            level->smell[y][x] = 0;
            level->items[y][x] = NULL;
        }
    }

    return level;
}

Mob *get_enemy(const Level *level, RL_Point coords)
{
    if (coords.y >= MAX_HEIGHT || coords.x >= MAX_WIDTH || coords.y < 0 || coords.x < 0)
        return NULL;

    for (int i = 0; i < MAX_MOBS; ++i)
    {
        if (level->mobs[i] == NULL)
            continue;
        if (level->mobs[i]->coords.x == coords.x && level->mobs[i]->coords.y == coords.y) {
            return level->mobs[i];
        }
    }

    return NULL;
}

Mob *get_mob(const Level *level, RL_Point coords)
{
    if (coords.y >= MAX_HEIGHT || coords.x >= MAX_WIDTH || coords.y < 0 || coords.x < 0)
        return NULL;

    if (level->player != NULL)
    {
        if (level->player->coords.x == coords.x && level->player->coords.y == coords.y)
            return level->player;
    }

    return get_enemy(level, coords);
}

// TODO handle NULL exception
int move_mob(Mob *mob, RL_Point coords, Level *level)
{
    if (mob == NULL)
        return 0;

    // check for mob
    Mob *target = get_mob(level, coords);

    if (target != NULL)
        return 0;

    if (rl_map_is_passable(level->map, coords))
    {
        mob->coords.x = coords.x;
        mob->coords.y = coords.y;

        return 1;
    }
    else
        return 0;
}
