#ifndef DUNGEON_H
#define DUNGEON_H

// map constants
#define MAX_LEVEL  10
#define MAX_WIDTH  80
#define MAX_HEIGHT 30

// for dungeon generator
#define MIN_CELLS 8
#define MAX_CELLS 12
#define MAX_RANDOM_RECURSION 1000

#define FOV_RAIDUS 8
#define MOB_ALERT_RADIUS FOV_RADIUS

// macro helper
#define DIRECTION(x, y) (Direction) {x, y}

#include "random.h"

#include "item.h"
#include "mob.h"
#include "lib/roguelike.h"

typedef struct Level_t {
    Mob *player;
    Mob *mobs[MAX_MOBS];
    RL_Map *map;
    RL_Heap *items[MAX_HEIGHT][MAX_WIDTH]; // game-specific tile data (items, mob, etc.)

    int depth;
    struct Level_t *prev;
    struct Level_t *next;
    RL_Point upstair_loc;
    RL_Point downstair_loc;
} Level;

typedef struct {
    Mob *player;
    Level *level;
    int turn; // turn number
    RL_Heap *killed; // mobs player has killed
} Dungeon;

typedef struct {
    int xdir;
    int ydir;
} Direction;

// create a new dungeon (once per game)
// returns NULL if there was any issues allocating memory for
// dungeon members
Dungeon *create_dungeon(unsigned long seed);

Level *create_level(int depth);

// get max dungeon depth
int max_depth(Dungeon *dungeon);

// initialize random dungeon
int init_level(Level *level, Mob *player);

// return random coordinates that are passable and do not have a mob
RL_Point random_passable_coords(Level *level);

Mob *get_mob(const Level *level, RL_Point coords);
Mob *get_enemy(const Level *level, RL_Point coords);
int move_mob(Mob *mob, RL_Point coords, Level *level);

#endif
