#ifndef ITEM_H
#define ITEM_H

#define ITEM_GOLD   0
#define ITEM_FOOD   1
#define ITEM_POTION 2
#define ITEM_SCROLL 3

#define ITEM_EFFECT_NONE 0
#define ITEM_EFFECT_HEAL 1
#define ITEM_EFFECT_TELEPORT 2
#define ITEM_EFFECT_DAMAGE 3

typedef struct {
    int id; // unique item ID for identification & index purposes
    int type; // one of ITEM consts
    int effect; // one of ITEM_EFFECT consts (for potions/scrolls)
    unsigned int strength; // strength of EFFECT or amount of $
    const char *unknownName; // random (unidentified) name
    const char *name; // real (identified) name
} Item;

#endif
