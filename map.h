#ifndef MAP_H
#define MAP_H

#define MAX_LEVEL  10
#define MAX_MOBS   20
#define MAX_WIDTH  80
#define MAX_HEIGHT 25

#include "item.h"
#include "mob.h"
#include "tile.h"
#include "path.h"

typedef struct Level_t {
    Mob **mobs;
    Tile **tiles; // 2d array of tiles
    struct Level_t *prev;
    struct Level_t *next;
    int depth;
} Level;

typedef struct {
    Mob *player;
    Level *level;
    const char **unknownItems; // randomized item names, indexed by ID
    const char **knownItems; // known item names, indexed by ID
    Mob **killed; // killed monsters (for scorekeeping)
} Dungeon;

// create a new dungeon (once per game)
// returns NULL if there was any issues allocating memory for
// dungeon members
Dungeon *create_dungeon();

// create a new dungeon level
// returns NULL if there was any issues allocating memory for
// level members
Level *create_level(int depth);

// get tile from square, or NULL on error
const Tile *get_tile(const Level *level, Coords coords);

// get mob from square, or NULL on error
Mob *get_mob(const Level *level, Coords coords);

// place mob on tile type (i.e. up or down stair)
int place_on_tile(Mob *mob, int tileType, const Level *level);

// move a mob to x, y
// return 1 on success, or 0 if impassable
int move_mob(Mob *mob, Coords coords, Level *level);

#endif
