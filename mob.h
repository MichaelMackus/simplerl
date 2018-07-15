#ifndef MOB_H
#define MOB_H

#define MOB_PLAYER 1
#define MOB_ENEMY  2
#define MOB_DRAGON 3
#define MOB_MIND_FLAYER 4

#include "item.h"

typedef struct {
    unsigned int x, y;
    unsigned int hp, maxHP; // TODO hp should be signed
    unsigned int minDamage, maxDamage;
    int type;
    char symbol;
    Item *items;
} Mob;

// return a random mob for the specified dungeon depth
Mob *createMob(int depth);

#endif
