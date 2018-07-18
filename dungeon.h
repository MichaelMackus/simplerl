#ifndef DUNGEON_H
#define DUNGEON_H

// for dungeon generator
#define MIN_CELLS 8
#define MAX_CELLS 12
#define MAX_RANDOM_RECURSION 100

#include "map.h"
#include "random.h"

typedef struct {
    Mob *player;
    Level *level;
    Mobs killed; // killed monsters (for scorekeeping)
    int turn; // turn number
} Dungeon;

// create a new dungeon (once per game)
// returns NULL if there was any issues allocating memory for
// dungeon members
Dungeon *create_dungeon();

// return random coordinates that are passable and
// do not have a mob
Coords random_passable_coords(Level *level);

// randomly generate tiles by filling dungeon with cells
// and then connecting via corridors
void randomly_fill_tiles(Level *level);

// randomly fill mobs in level by max amount
void randomly_fill_mobs(Level *level, int max);

#endif
