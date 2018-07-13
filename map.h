#ifndef MAP_H
#define MAP_H

#define MAX_LEVEL 10
#define MAX_MOBS  20

#include "mob.h"

typedef struct Level_t {
    Mob *mobs;
    struct Level_t *prev;
    struct Level_t *next;
    int depth;
} Level;

typedef struct {
    Mob *player;
    Level *level;
} Dungeon;

// create a new dungeon (once per game)
// returns NULL if there was any issues allocating memory for
// dungeon members
Dungeon *create_dungeon();

// create a new dungeon level
// returns NULL if there was any issues allocating memory for
// level members
Level *create_level(int depth);

// change current depth to next level deep
// if there is no next level, create one
void increase_depth(Dungeon dungeon);

// change current depth to previous level
void decrease_depth(Dungeon dungeon);

#endif
