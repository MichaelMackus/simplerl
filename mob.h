#ifndef MOB_H
#define MOB_H

#define MAX_MOBS   20
#define MAX_PLAYER_LEVEL 10

#define MOB_PLAYER 1
#define MOB_ENEMY  2
#define MOB_DRAGON 3
#define MOB_MIND_FLAYER 4
#define MOB_DEMON 5

#define MENU_INVENTORY 1
#define MENU_WIELD 2
#define MENU_WEAR 3

#include "item.h"
#include "path.h"

typedef struct {
    int resting; // boolean
    Direction running; // direction player is running, or (0,0)
    int inMenu; // one of MENU consts if in menu
    int exp;
    int level;
} PlayerAttributes;

typedef struct {
    Item *weapon;
    Item *armor;
} Equipment;

typedef struct {
    int hp, maxHP;
    Coords coords;
    unsigned int minDamage, maxDamage;
    int type;
    char symbol;
    Items items;
    Equipment equipment;
    union {
        PlayerAttributes attrs;
        int difficulty;
    };
} Mob;

typedef struct {
    size_t count;
    size_t size;
    Mob **content;
} Mobs;

// return a random mob for the specified dungeon depth
Mob *create_mob(int depth, Coords coords);

// try to attack x, y
// if no mob found at x, y do nothing
// return damage
int attack(Mob *attacker, Mob *target);

// insert mob into mobs list
void insert_mob(Mob *mob, Mob **mobs);

// insert into killed mobs array
int kill_mob(Mob *mob, Mobs *mobs);

// allocate mobs list
Mobs initialize_mobs();

// return mob name for symbol
const char* mob_name(char symbol);

// helper functions to determine player status

int is_resting(Mob *player);
int is_running(Mob *player);
int in_menu(Mob *player);

#endif
