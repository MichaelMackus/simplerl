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
        m = enemy(5, 4, 7, 'o'); // orc
    else if (difficulty <= 7)
        m = enemy(10, 7, 10, 'O'); // ogre
    else if (difficulty <= 9)
        m = enemy(12, 10, 12, 'd'); // drake
    else if (difficulty <= 11)
    {
        m = enemy(12, 10, 12, 'H'); // mind flayer
        m->type = MOB_MIND_FLAYER;
        // TODO drain effect
    }
    else if (difficulty <= 15)
    {
        m = enemy(20, 13, 17, 'D'); // dragon
        m->type = MOB_DRAGON;
        // TODO breath effects
    }
    else
    {
        m = enemy(30, 15, 20, '&'); // demon
        m->type = MOB_DEMON;
        // TODO drain effects
    }

    // OOM check
    if (m == NULL)
        return NULL;

    // give mob some gold and a weapon
    move_item(create_item(depth, ITEM_GOLD), &m->items);
    move_item(create_item(depth, ITEM_WEAPON), &m->items);
    move_item(create_item(depth, ITEM_ARMOR), &m->items);

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

    int damage;
    if (attacker->equipment.weapon != NULL)
    {
        // calculate based on equipped weapon
        damage = generate(
                attacker->equipment.weapon->damage.min,
                attacker->equipment.weapon->damage.max);
    }
    else
        damage = generate(attacker->minDamage, attacker->maxDamage);

    // calculate DR based on equipped armor
    if (target->equipment.armor != NULL)
    {
        damage -= target->equipment.armor->armor.damageReduction;
        // always hit for a minimum of 1 damage
        if (damage <= 0) damage = 1;
    }

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
    m->equipment.weapon = NULL;
    m->equipment.armor = NULL;

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

int kill_mob(Mob *mob, Mobs *mobs)
{
    if (mob == NULL)
        return 1;

    size_t count = mobs->count;
    size_t size = mobs->size;

    if (count >= size)
    {
        Mob **tmp = realloc(mobs->content, sizeof(Mob*) * (size + MAX_MOBS));

        if (tmp == NULL)
            return 0;

        mobs->size += MAX_MOBS;
        mobs->content = tmp;
    }

    mobs->content[count] = mob;
    ++mobs->count;

    return 1;
}

Mobs initialize_mobs()
{
    Mobs mobs;
    mobs.size = mobs.count = 0;
    mobs.content = NULL;

    return mobs;
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

int in_menu(Mob *player)
{
    if (player == NULL || player->type != MOB_PLAYER)
        return 0;

    return player->attrs.inMenu > 0;
}

const char* mob_name(char symbol)
{
    switch (symbol)
    {
        case 'r':
            return "rat";
        case 'k':
            return "kobold";
        case 'g':
            return "goblin";
        case 'o':
            return "orc";
        case 'O':
            return "ogre";
        case 'd':
            return "drake";
        case 'H':
            return "mind flayer";
        case 'D':
            return "dragon";
        case '&':
            return "demon";

        default:
            return "unknown";
    }
}
