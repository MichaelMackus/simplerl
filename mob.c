#include "mob.h"
#include "path.h"
#include "random.h"
#include <stdlib.h>

Mob *enemy(unsigned int hp, unsigned int minDamage, unsigned int maxDamage, char symbol);
Mob *create_mob(int depth, Coords coords)
{
    // difficulty ranges
    //
    // level 1: 1 - 2
    // level 2: 2 - 4
    // level 3: 3 - 6
    // level 5: 5 - 10
    // level 7: 7 - 14
    // level 10: 10 - 20
    int difficulty = generate(depth, depth*2);

    Mob *m;

    if (difficulty == 1)
        m = enemy(4, 1, 2, 'r'); // rat
    else if (difficulty == 2)
        m = enemy(4, 2, 3, 'k'); // kobold
    else if (difficulty == 3)
        m = enemy(5, 3, 5, 'g'); // goblin
    else if (difficulty <= 5)
        m = enemy(5, 3, 5, 'o'); // orc
    else if (difficulty <= 7)
        m = enemy(10, 6, 9, 'O'); // ogre
    else if (difficulty <= 9)
        m = enemy(12, 7, 10, 'd'); // drake
    else if (difficulty <= 11)
    {
        m = enemy(12, 7, 10, 'H'); // mind flayer
        m->type = MOB_MIND_FLAYER;
        // TODO drain effect
    }
    else if (difficulty <= 15)
    {
        m = enemy(20, 10, 15, 'D'); // dragon
        m->type = MOB_DRAGON;
        // TODO breath effects
    }
    else
    {
        m = enemy(30, 13, 17, '&'); // demon
        m->type = MOB_DEMON;
        // TODO drain effects
    }

    // OOM check
    if (m == NULL)
        return NULL;

    // give mob some gold and a weapon
    insert_item(create_item(depth, ITEM_GOLD), &m->items);
    insert_item(create_item(depth, ITEM_WEAPON), &m->items);

    m->difficulty = difficulty;
    m->coords = coords;

    return m;
}

// free mob & items
void free_mob(Mob *mob)
{
    free_items(mob->items);
    free(mob);
}

// try to attack x, y
// if no mob found at x, y do nothing
int attack(Mob *attacker, Mob *target)
{
    if (attacker == NULL || target == NULL)
        return 0;

    // calculate damage based on attacker's stats
    int damage = generate(attacker->minDamage, attacker->maxDamage);

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
    m->items = initialize_items();

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

int is_resting(Mob *player)
{
    if (player == NULL || player->type != MOB_PLAYER)
        return 0;

    return player->attrs.resting;
}

int is_running(Mob *player)
{
    if (player == NULL || player->type != MOB_PLAYER)
        return 0;

    Direction runDir = player->attrs.running;

    return runDir.xdir != 0 || runDir.ydir != 0;
}
