#include "mob.h"
#include "random.h"
#include <stdlib.h>

Mob *enemy(unsigned int hp, unsigned int minDamage, unsigned int maxDamage, char symbol, int form);
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
        m = enemy(4, 1, 2, 'r', MOB_FORM_QUADRAPED); // rat
    else if (difficulty == 2)
        m = enemy(4, 2, 3, 'k', MOB_FORM_BIPED); // kobold
    else if (difficulty == 3)
        m = enemy(5, 2, 3, 'g', MOB_FORM_BIPED); // goblin
    else if (difficulty <= 5)
        m = enemy(6, 2, 4, 'o', MOB_FORM_BIPED); // orc
    else if (difficulty <= 7)
        m = enemy(8, 3, 4, 'h', MOB_FORM_BIPED); // hobgoblin
    else if (difficulty <= 9)
        m = enemy(12, 4, 8, 'O', MOB_FORM_BIPED); // ogre
    else if (difficulty <= 14)
        m = enemy(15, 8, 12, 'd', MOB_FORM_QUADRAPED & MOB_FORM_FLYING); // drake
    else if (difficulty <= 18)
    {
        m = enemy(20, 12, 15, 'D', MOB_FORM_QUADRAPED & MOB_FORM_FLYING); // dragon

        // OOM check
        if (m == NULL)
            return NULL;

        m->type = MOB_DRAGON;
        // TODO breath effects
        // TODO dragons always have dragonhide
    }
    else
    {
        m = enemy(30, 15, 20, '&', MOB_FORM_BIPED & MOB_FORM_FLYING); // demon

        // OOM check
        if (m == NULL)
            return NULL;

        m->type = MOB_DEMON;
        // TODO drain effects
        // TODO demons always have cool sword
    }

    // OOM check
    if (m == NULL)
        return NULL;

    if (m->form & MOB_FORM_BIPED)
    {
        // give mob some gold
        move_item(create_item(depth, ITEM_GOLD), &m->items);

        // give mob default weapon
        if (m->equipment.weapon == NULL)
        {
            // difficulty * 10 percent chance of weapon
            if (generate(1, 100) <= difficulty*10)
            {
                Item *item = create_item(depth, ITEM_WEAPON);

                // OOM check
                if (m == NULL)
                    return NULL;

                m->equipment.weapon = move_item(item, &m->items);
            }
        }

        // give mob default armor
        if (m->equipment.armor == NULL)
        {
            // difficulty * 5 percent chance of armor
            if (generate(1, 100) <= difficulty*5)
            {
                Item *item = create_item(depth, ITEM_ARMOR);

                // OOM check
                if (m == NULL)
                    return NULL;

                m->equipment.armor = move_item(item, &m->items);
            }
        }
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

Mob *enemy(unsigned int hp, unsigned int minDamage, unsigned int maxDamage, char symbol, int form)
{
    Mob *m;
    m = malloc(sizeof(Mob));

    m->hp = hp;
    m->maxHP = hp;
    m->minDamage = minDamage;
    m->maxDamage = maxDamage;
    m->symbol = symbol;
    m->type = MOB_ENEMY;
    m->form = form;
    m->items = initialize_items();
    m->equipment.weapon = NULL;
    m->equipment.armor = NULL;

    return m;
}

int insert_mob(Mob *mob, Mob **mobs)
{
    // seek to available mob index
    int mobIndex = 0;
    while (mobs[mobIndex] != NULL)
    {
        ++mobIndex;
        if (mobIndex >= MAX_MOBS)
            return 0; // out of range!
    }

    mobs[mobIndex] = mob;

    return 1;
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

const char* mob_name(char symbol)
{
    switch (symbol)
    {
        // TODO small weapon(s)
        case 'k':
            return "kobold";
        case 'g':
            return "goblin";
        case 'o':
            return "orc";
        case 'h':
            return "hobgoblin";
        // TODO large weapon, maybe boulder(s)
        case 'O':
            return "ogre";
        // TODO no weapons *or* pockets
        case 'r':
            return "rat";
        case 'd':
            return "drake";
        // TODO best weapon(s)
        case '&':
            return "demon";
        // TODO no weapons, lots of gold, possibly other items
        case 'H':
            return "mind flayer";
        case 'D':
            return "dragon"; // TODO bonus gold

        default:
            return "unknown";
    }
}
