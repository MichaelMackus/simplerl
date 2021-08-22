#include "dungeon.h"
#include <lib/bsp.h>
#include <lib/mapgen.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

typedef struct {
    int xdir;
    int ydir;
} Direction;

Dungeon *create_dungeon()
{
    // do this otherwise initial seed will always be the same
    init_random();
    seed_random();

    // allocate dungeon
    Dungeon *dungeon;
    dungeon = malloc(sizeof(Dungeon));

    // handle out of memory case
    if (dungeon == NULL)
        return NULL;

    dungeon->turn = 0;

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

    // randomly populate *new* levels with max of MAX_MOBS / 2
    randomly_fill_mobs(level, MAX_MOBS / 2);

    // place player on upstair
    player->coords = level->upstair_loc;

    // set smell around player before movement (also done in move_player)
    taint(player->coords, level);

    return 1;
}

// taint smell aura around player
void taint(const rl_coords playerCoords, Level *level)
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
        rl_coords location = RL_XY(coords[i][0], coords[i][1]);
        int smell = coords[i][2];

        // out of bounds, skip coord
        if (!rl_in_map_bounds(level->map, location)) continue;

        // set smell if tile & greater than current smell
        if (level->tiles[location.y][location.x].smell < smell)
            level->tiles[location.y][location.x].smell = smell;
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
    rl_bsp *bsp = rl_create_bsp(MAX_WIDTH, MAX_HEIGHT);
    if (bsp == NULL) return;
    rl_recursively_split_bsp(bsp, &rl_rng_twister_generate, 6, 6, 0.5, 4);
    level->map = rl_create_map_from_bsp(bsp, &rl_rng_twister_generate, 4, 4, 8, 8, 1);
    if (level->map == NULL) return;

    // randomize corridor generation
    if (generate(0, 1))
        rl_connect_corridors_chaotic(level->map, bsp, &rl_rng_twister_generate);
    else
        rl_connect_corridors_to_random_siblings(level->map, bsp, &rl_rng_twister_generate);

    // populate our internal tiles array
    for (int x = 0; x < MAX_WIDTH; ++x) {
        for (int y = 0; y < MAX_HEIGHT; ++y) {
            level->tiles[y][x] = (Tile) {0};
            level->tiles[y][x].type = rl_get_tile(level->map, RL_XY(x, y));
        }
    }

    // randomly place upstairs
    rl_coords up;
    int i = 0;
    while (i < MAX_RANDOM_RECURSION) {
        up = random_passable_coords(level);
        if (rl_is_room(level->map, up)) {
            level->upstair_loc = up;
            break;
        }
    }

    // randomly place downstairs
    // TODO place downstairs at greater distance from upstairs
    // TODO once win condition is defined, don't place downstairs on last level
    rl_coords down;
    i = 0;
    while (i < MAX_RANDOM_RECURSION) {
        down = random_passable_coords(level);
        if (rl_is_room(level->map, down)) {
            level->downstair_loc = down;
            break;
        }
    }

    rl_free_bsp(bsp);
}

void randomly_fill_mobs(Level *level, int max)
{
    Mob **mobs = level->mobs;

    int mobIndex = 0;
    int amount = generate(0, max);
    for (int i = 0; i < amount; ++i)
    {
        rl_coords coords = random_passable_coords(level);
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

rl_coords random_coords(Level *level)
{
    rl_coords coords;
    coords.x = generate(0, MAX_WIDTH - 1);
    coords.y = generate(0, MAX_HEIGHT - 1);

    return coords;
}

// FIXME these two functions are a quick fix
rl_coords empty_coords()
{
    rl_coords coords;
    coords.x = MAX_WIDTH;
    coords.y = MAX_HEIGHT;

    return coords;
}

Mob *get_mob(const Level *level, rl_coords coords);
rl_coords random_passable_coords(Level *level)
{
    // do simple brute force attempt to get a passable coord
    int i = 0;
    while (i < MAX_RANDOM_RECURSION)
    {
        rl_coords coords = random_coords(level);
        Mob *m = get_mob(level, coords);
        if (rl_is_passable(level->map, coords) && m == NULL)
            return coords;
        ++i;
    }

    return empty_coords();
}

Level *create_level(int depth)
{
    Level *level;
    level = malloc(sizeof(Level));

    // check for OOM
    if (level == NULL)
        return NULL;

    // initialize mobs & tiles array
    for (int i = 0; i < MAX_MOBS; ++i) level->mobs[i] = NULL;
    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            level->tiles[y][x] = (Tile){0};
        }
    }

    // initialize depth
    level->depth = depth;

    // initialize vars to null
    level->next = NULL;
    level->prev = NULL;

    return level;
}

Mob *get_enemy(const Level *level, rl_coords coords)
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

Mob *get_mob(const Level *level, rl_coords coords)
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
int move_mob(Mob *mob, rl_coords coords, Level *level)
{
    if (mob == NULL)
        return 0;

    // check for mob
    Mob *target = get_mob(level, coords);

    if (target != NULL)
        return 0;

    if (rl_is_passable(level->map, coords))
    {
        mob->coords.x = coords.x;
        mob->coords.y = coords.y;

        return 1;
    }
    else
        return 0;
}

int can_see(rl_coords from, rl_coords to, Tile tiles[MAX_HEIGHT][MAX_WIDTH])
{
    int ret = 0;
    int length = 0;
    rl_path *path = rl_get_line(from, to);
    const rl_coords *loc;
    while ((loc = rl_walk_path(path)) != NULL)
    {
        // if the line ends at the point we're looking at, we can see it!
        if (loc->x == to.x && loc->y == to.y)
        {
            ret = 1;
            break;
        }

        // if the current coord blocks the view and is at least 1 space
        // away, we can't see it
        if (loc->x < MAX_WIDTH && loc->y < MAX_HEIGHT && loc->x >= 0 && loc->y >= 0) {
            rl_tile type = tiles[loc->y][loc->x].type;
            if ((type == RL_TILE_BLOCK || type == RL_TILE_WALL || type == RL_TILE_PASSAGE) && length)
                break;
        }

        ++length;
    }
    rl_clear_path(path);

    return ret;
}

