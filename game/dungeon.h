#ifndef DUNGEON_H
#define DUNGEON_H

// map constants
#define MAX_LEVEL  10
#define MAX_WIDTH  80
#define MAX_HEIGHT 25
#define INITIAL_SMELL 10

// for dungeon generator
#define MIN_CELLS 8
#define MAX_CELLS 12
#define MAX_RANDOM_RECURSION 100

// macro helper
#define DIRECTION(x, y) (Direction) {x, y}

#include "random.h"

#include "item.h"
#include "mob.h"
#include <lib/path.h>
#include <lib/map.h>
#include <lib/queue.h>

typedef struct {
    rl_queue *items;
    rl_tile type; // underlying tile type
    int seen;     // seen by player?
    int smell;    // when player passes onto tile, set this to INITIAL_SMELL and decrement each turn
} Tile;

typedef struct Level_t {
    Mob *player;
    Mob *mobs[MAX_MOBS];
    rl_map *map;
    Tile tiles[MAX_HEIGHT][MAX_WIDTH]; // game-specific tile data (items, mob, etc.)

    int depth;
    struct Level_t *prev;
    struct Level_t *next;
    rl_coords upstair_loc;
    rl_coords downstair_loc;
} Level;

typedef struct {
    Mob *player;
    Level *level;
    int turn; // turn number
    rl_queue *killed; // mobs player has killed
} Dungeon;

typedef struct {
    int xdir;
    int ydir;
} Direction;

// create a new dungeon (once per game)
// returns NULL if there was any issues allocating memory for
// dungeon members
Dungeon *create_dungeon();

Level *create_level(int depth);

// get max dungeon depth
int max_depth(Dungeon *dungeon);

// initialize random dungeon
int init_level(Level *level, Mob *player);

// taint tiles around player (for AI pathfinding)
void taint(const rl_coords playerCoords, Level *level);

// return random coordinates that are passable and do not have a mob
rl_coords random_passable_coords(Level *level);

// simple lighting function to see if mob can see x and y
int can_see(rl_coords from, rl_coords to, Tile tiles[MAX_HEIGHT][MAX_WIDTH]);

Mob *get_mob(const Level *level, rl_coords coords);
Mob *get_enemy(const Level *level, rl_coords coords);
char get_symbol(Level *level, rl_coords coords);
int move_mob(Mob *mob, rl_coords coords, Level *level);

#endif
