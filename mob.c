#include "mob.h"
#include "path.h"
#include <stdlib.h>

Mob *enemy(unsigned int hp, unsigned int minDamage, unsigned int maxDamage, char symbol);
Mob *createMob(int depth, Coords coords)
{
    // difficulty ranges
    //
    // level 1: 1 - 2
    // level 2: 2 - 4
    // level 3: 3 - 5
    // level 5: 5 - 11
    // level 7: 7 - 13
    // level 10: 10 - 19
    int difficulty = (rand() % (depth+1)) + depth;

    Mob *m;

    // TODO need to scale difficulty more later (i.e. hobgoblins in later dungeon...)
    if (difficulty == 1)
        m = enemy(4, 1, 2, 'r'); // rat
    else if (difficulty == 2)
        m = enemy(4, 2, 3, 'k'); // kobold
    else if (difficulty == 3)
        m = enemy(5, 3, 5, 'g'); // goblin
    else if (difficulty <= 5)
        m = enemy(5, 3, 5, 'o'); // orc
    else if (difficulty <= 7)
        m = enemy(7, 5, 7, 'h'); // hobgoblin
    else if (difficulty <= 9)
        m = enemy(10, 6, 9, 'O'); // ogre
    else if (difficulty <= 11)
        m = enemy(12, 7, 10, 'd'); // drake
    else if (difficulty <= 13)
    {
        m = enemy(12, 7, 10, 'H'); // mind flayer

        // OOM check
        if (m == NULL)
            return NULL;

        m->type = MOB_MIND_FLAYER;
        // TODO drain effect
    }
    else //if (difficulty >= 14)
    {
        m = enemy(20, 10, 15, 'D'); // dragon

        // OOM check
        if (m == NULL)
            return NULL;

        m->type = MOB_DRAGON;
        // TODO breath effects
    }

    m->difficulty = difficulty;
    m->coords = coords;

    return m;
}

// try to attack x, y
// if no mob found at x, y do nothing
int attack(Mob *attacker, Mob *target)
{
    if (attacker == NULL || target == NULL)
        return 0;

    // calculate damage based on attacker's stats
    // TODO add simple roll function
    int damage = rand() % (attacker->maxDamage - attacker->minDamage + 1)  +  attacker->minDamage;

    target->hp -= damage;

    return damage;
}

Mob *enemy(unsigned int hp, unsigned int minDamage, unsigned int maxDamage, char symbol)
{
    Mob *m;
    m = malloc(sizeof(Mob));

    m->hp = hp;
    m->maxHP = hp;
    m->minDamage = minDamage;
    m->maxDamage = maxDamage;
    m->symbol = symbol;
    m->type = MOB_ENEMY;
    m->items = NULL;

    return m;
}

void insert_mob(Mob *mob, Mob **mobs)
{
    // seek to available mob index
    int mobIndex = 0;
    while (mobs[mobIndex] != NULL)
    {
        ++mobIndex;
        if (mobIndex >= MAX_MOBS)
            return; // out of range!
    }

    mobs[mobIndex] = mob;
}
