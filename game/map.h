#ifndef MAP_H
#define MAP_H

#define MAX_LEVEL  10
#define MAX_WIDTH  80
#define MAX_HEIGHT 25

#define XY(x,y) (rl_coords){x,y}

#include "item.h"
#include "mob.h"
#include "tile.h"
#include <lib/path.h>

typedef struct Level_t {
    Mob *player;
    Mob **mobs;
    Tile **tiles; // 2d array of tiles
    struct Level_t *prev;
    struct Level_t *next;
    int depth;
} Level;

// create a new dungeon level
// returns NULL if there was any issues allocating memory for
// level members
Level *create_level(int depth);

// get tile from square, or NULL on error
Tile *get_tile(const Level *level, rl_coords coords);

// get enemy from square, or NULL on error
Mob *get_enemy(const Level *level, rl_coords coords);

// get enemy *or* player from square, or NULL on error
Mob *get_mob(const Level *level, rl_coords coords);

// place mob on tile type (i.e. up or down stair)
int place_on_tile(Mob *mob, int tileType, const Level *level);

// move a mob to x, y
// return 1 on success, or 0 if impassable
int move_mob(Mob *mob, rl_coords coords, Level *level);

#endif
