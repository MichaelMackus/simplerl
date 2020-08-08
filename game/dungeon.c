#include "dungeon.h"
#include <lib/bsp.h>
#include <lib/mapgen.h>
#include <stdlib.h>
#include <memory.h>

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

    // allocate killed list
    dungeon->killed = initialize_mobs();

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
    player->equipment.weapon = NULL;
    player->equipment.armor = NULL;
    player->attrs.exp = 0;
    player->attrs.expNext = 1000;
    player->attrs.level = 1;

    // give player some simple equipment
    Item *armor = leather();
    Item *weapon = quarterstaff();
    player->equipment.armor = move_item(armor, &player->items);
    player->equipment.weapon = move_item(weapon, &player->items);

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
        ++cur;

    return cur->depth;
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
    rl_map *map = rl_create_map_from_bsp(bsp, &rl_rng_twister_generate, 4, 4, 8, 8, 1);
    if (map == NULL) return;
    rl_connect_corridors_to_random_siblings(map, bsp, &rl_rng_twister_generate, 1);

    // transfer the RL map to our custom map
    for (int x = 0; x < MAX_WIDTH; ++x) {
        for (int y = 0; y < MAX_HEIGHT; ++y) {
            rl_tile tile = rl_get_tile(map, XY(x, y));
            int custom_tile = 0;
            switch (tile) {
                case RL_TILE_WALL:
                    if (rl_is_wall(map, XY(x + 1, y)) || rl_is_wall(map, XY(x - 1, y)) ||
                            rl_is_doorway(map, XY(x + 1, y)) || rl_is_doorway(map, XY(x - 1, y)))
                        custom_tile = TILE_WALL;
                    else
                        custom_tile = TILE_WALL_SIDE;
                    break;
                case RL_TILE_ROOM:
                    custom_tile = TILE_FLOOR;
                    break;
                case RL_TILE_PASSAGE:
                    custom_tile = TILE_CAVERN;
                    break;
                case RL_TILE_DOORWAY:
                    custom_tile = TILE_FLOOR;
                    break;
                default:
                    break;
            }

            level->tiles[y][x].type = custom_tile;
        }
    }
    rl_free_map(map);
    rl_free_bsp(bsp);

    // randomly place upstairs
    rl_coords up;
    int i = 0;
    while (i < MAX_RANDOM_RECURSION) {
        up = random_passable_coords(level);
        if (level->tiles[up.y][up.x].type == TILE_FLOOR) {
            level->tiles[up.y][up.x].type = TILE_STAIR_UP;
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
        if (level->tiles[down.y][down.x].type == TILE_FLOOR) {
            level->tiles[down.y][down.x].type = TILE_STAIR_DOWN;
            break;
        }
    }
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
        while (get_tile(level, coords)->type == TILE_STAIR_UP ||
                get_tile(level, coords)->type == TILE_STAIR_DOWN)
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

rl_coords random_open_coords(Level *level)
{
    // do simple brute force attempt to get an open coord
    int i = 0;
    while (i < MAX_RANDOM_RECURSION)
    {
        rl_coords coords = random_coords(level);
        const Tile *t = get_tile(level, coords);
        if (t != NULL && t->type == TILE_NONE)
            return coords;
        ++i;
    }

    return empty_coords();
}

rl_coords random_passable_coords(Level *level)
{
    // do simple brute force attempt to get a passable coord
    int i = 0;
    while (i < MAX_RANDOM_RECURSION)
    {
        rl_coords coords = random_coords(level);
        const Tile *t = get_tile(level, coords);
        Mob *m = get_mob(level, coords);
        if (t != NULL && is_passable(*t) && m == NULL)
            return coords;
        ++i;
    }

    return empty_coords();
}
