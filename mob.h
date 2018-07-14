#ifndef MOB_H
#define MOB_H

#define MOB_PLAYER 1
#define MOB_ENEMY  2
#define MOB_NPC    3

#include "item.h"

typedef struct {
    unsigned int x, y;
    int type;
    char symbol;
    Item *items;
} Mob;

#endif
