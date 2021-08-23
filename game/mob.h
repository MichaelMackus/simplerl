#ifndef MOB_H
#define MOB_H

#define MAX_MOBS            20
#define MAX_PLAYER_LEVEL    20
#define MAX_INVENTORY_ITEMS 27 // 1 spot for gold + 26 inventory letters

#define MOB_PLAYER 1
#define MOB_ENEMY  2
#define MOB_DRAGON 3
#define MOB_MIND_FLAYER 4
#define MOB_DEMON 5

#define MOB_FORM_BIPED 1
#define MOB_FORM_QUADRAPED 2
#define MOB_FORM_FLYING 4

#include "item.h"
#include <lib/path.h>

typedef struct {
    int exp;
    int expNext;
    int level;
} PlayerAttributes;

typedef struct {
    Item *weapon;
    Item *armor;
    Item *readied; // readied item to throw/fire
} Equipment;

typedef struct {
    int hp, maxHP;
    rl_coords coords;
    int minDamage, maxDamage;
    int type;
    int form;
    char symbol;
    Item *items[MAX_INVENTORY_ITEMS];
    int itemCount;
    Equipment equipment;
    union {
        PlayerAttributes attrs;
        int difficulty; // TODO use this for exp
    };
} Mob;

// return a random mob for the specified dungeon depth
Mob *create_mob(int depth, rl_coords coords);

// try to attack x, y
// if no mob found at x, y do nothing
// return damage
int attack(Mob *attacker, Mob *target, Item *weapon);

// insert mob into mobs list
int insert_mob(Mob *mob, Mob **mobs);

// return mob name for symbol
const char* mob_name(char symbol);

// give item to mob
int give_mob_item(Mob *mob, Item *item);

// decrement item from mob (this decrements quantity by 1 if >1)
// returns 1 if item has been removed from mob inventory, 
// or -1 if amount of item in inventory decremented (0 on error)
int decrement_mob_item(Mob *mob, Item *item);

// remove item from mob entirely
int remove_mob_item(Mob *mob, Item *item);

#endif
