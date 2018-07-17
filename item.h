#ifndef ITEM_H
#define ITEM_H

#include <stdlib.h>

#define ITEM_GOLD   0
#define ITEM_WEAPON 1
#define ITEM_POTION 2
#define ITEM_SCROLL 3

#define DAMAGE_BLUNT 0
#define DAMAGE_SLASH 1
#define DAMAGE_SILVER 2

#define ITEM_EFFECT_NONE 0
#define ITEM_EFFECT_HEAL 1
#define ITEM_EFFECT_TELEPORT 2
#define ITEM_EFFECT_DAMAGE 3

// for potions & scrolls
typedef struct {
    int effect; // one of ITEM_EFFECT consts
    unsigned int strength; // strength of EFFECT
} ItemAttributes;

// for weapons
typedef struct {
    int min;
    int max;
    int type; // one of DAMAGE consts
    int twoHanded; // boolean if this is two handed
} WeaponAttributes;

typedef struct {
    int type; // one of ITEM consts
    const char *name; // real (identified) name
    const char *unknownName; // random (unidentified) name
    union {
        int amount;
        ItemAttributes attributes;
        WeaponAttributes damage;
    };
} Item;

typedef struct {
    size_t count;
    size_t size;
    Item **content;
} Items;

const char **unknownItems;
const char **knownItems;

// return a random item for the specified dungeon depth
Item *create_item(int depth, int type);

// initialize our item list
Items initialize_items();

// insert item into variable-length list
int insert_item(Item *item, Items *items);

// TODO need to sort items by type for inventory management

#endif
