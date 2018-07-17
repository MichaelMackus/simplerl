#ifndef MOB_H
#define MOB_H

#define MAX_PLAYER_LEVEL 10

#define MOB_PLAYER 1
#define MOB_ENEMY  2
#define MOB_DRAGON 3
#define MOB_MIND_FLAYER 4

#include "item.h"
#include "tile.h"
#include "path.h"

typedef struct {
    int resting; // boolean
    int exp;
    int level;
} PlayerAttributes;

typedef struct {
    int hp, maxHP;
    Coords coords;
    unsigned int minDamage, maxDamage;
    int type;
    char symbol;
    Item *items;
    union {
        PlayerAttributes attrs;
        int difficulty;
    };
} Mob;

// return a random mob for the specified dungeon depth
Mob *createMob(int depth);

// try to attack x, y
// if no mob found at x, y do nothing
// return damage
int attack(Mob *attacker, Mob *target);

#endif
