#ifndef DUNGEON_H
#define DUNGEON_H

#include "map.h"

typedef struct {
    Mob *player;
    Level *level;
    const char **unknownItems; // randomized item names, indexed by ID
    const char **knownItems; // known item names, indexed by ID
    Mob **killed; // killed monsters (for scorekeeping)
    int turn; // turn number
} Dungeon;

// create a new dungeon (once per game)
// returns NULL if there was any issues allocating memory for
// dungeon members
Dungeon *create_dungeon();

#endif
