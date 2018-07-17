#ifndef ITEM_H
#define ITEM_H

#include <stdlib.h>

#define ITEM_GOLD   0
#define ITEM_FOOD   1
#define ITEM_POTION 2
#define ITEM_SCROLL 3

#define ITEM_EFFECT_NONE 0
#define ITEM_EFFECT_HEAL 1
#define ITEM_EFFECT_TELEPORT 2
#define ITEM_EFFECT_DAMAGE 3

typedef struct {
    int type; // one of ITEM consts
    int effect; // one of ITEM_EFFECT consts (for potions/scrolls)
    unsigned int strength; // strength of EFFECT or amount of $
    const char *unknownName; // random (unidentified) name
    const char *name; // real (identified) name
} Item;

typedef struct {
    size_t count;
    size_t size;
    Item **content;
} Items;

// return a random item for the specified dungeon depth
Item *create_item(int depth, int type, const char **unknownItems, const char **knownItems);

// initialize our item list
Items initialize_items();

// insert item into variable-length list
int insert_item(Item *item, Items *items);

// TODO need to sort items by type for inventory management

#endif
