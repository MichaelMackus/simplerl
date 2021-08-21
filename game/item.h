#ifndef ITEM_H
#define ITEM_H

#include <stdlib.h>

#define ITEM_GOLD   0
#define ITEM_WEAPON 1
#define ITEM_PROJECTILE 2
#define ITEM_ARMOR  3
#define ITEM_POTION 4
#define ITEM_SCROLL 5
#define ITEM_ROCK   64 | 2

// weapon damage types
#define WEAPON_BLUNT 0
#define WEAPON_SLASH 1
#define WEAPON_PIERCE 2
#define WEAPON_SILVER 3
// weapon flags
#define WEAPON_RANGED 64
#define WEAPON_TWOHANDED 128

#define MATERIAL_METAL 0
#define MATERIAL_LEATHER 1
#define MATERIAL_DRAGON 2

#define ITEM_EFFECT_NONE 0
#define ITEM_EFFECT_HEAL 1
#define ITEM_EFFECT_TELEPORT 2
#define ITEM_EFFECT_DAMAGE 3

#define PROJECTILE_ARROW 1

// for potions & scrolls
typedef struct {
    int effect; // one of ITEM_EFFECT consts
    unsigned int strength; // strength of EFFECT
} ItemAttributes;

// for weapons
typedef struct {
    int min;
    int max;
    int type; // one of WEAPON damage types anded with weapon flags
    int projectile; // one of PROJECTILE consts (for ranged weapons)
    int range; // range for projectile
} WeaponAttributes;

// for armor
typedef struct {
    int damageReduction; // simple damage reduction value
    int material; // one of MATERIAL consts
} ArmorAttributes;

typedef struct {
    int type; // one of ITEM consts
    int amount; // amount of items
    const char *name; // real (identified) name
    const char *unknownName; // random (unidentified) name
    union {
        ItemAttributes attributes;
        WeaponAttributes damage;
        ArmorAttributes armor;
    };
} Item;

// TODO remove this and replace with priority queue
// TODO can sort priority by A-Z index order
typedef struct {
    size_t count;
    size_t size;
    Item **content;
} Items;

char item_symbol(int itemType);
char inventory_symbol(const Item *item, Items items); // signifies selection spot in inventory

// calculate total amount of gold in inventory
int total_gold(Items items);

// return a random item for the specified dungeon depth
Item *create_item(int depth, int type);

// initialize our item list
Items initialize_items();

// insert item into variable-length list
// *NOTE*: this will free the item if it already exists, hence "move"
Item *move_item(Item *item, Items *items);

// take top item from list
Item *take_item(Items *items);

// copy item contents from one container to another
// *NOTE*: this will free the item(s) from src if they already exists, hence "move"
int move_items(Items *src, Items *target);

// free our item list
void free_items(Items items);

// TODO only stack certain items?
/* int is_stackable(Item item); */

// TODO need to sort items by type for inventory management

// specific armor generation functions
Item *leather();
Item *ring_mail();
Item *splint_mail();
Item *plate_mail();
Item *full_plate();
Item *dragon_hide();
Item *dragon_plate();

// specific weapon generation functions
Item *club();
Item *dagger();
Item *short_sword();
Item *mace();
Item *quarterstaff();
Item *long_sword();
Item *bastard_sword();
Item *flail();
Item *masterwork_sword();
Item *masterwork_bastard_sword();
Item *silver_sword();

#endif
