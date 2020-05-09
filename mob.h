#ifndef MOB_H
#define MOB_H

#define MAX_MOBS   20
#define MAX_PLAYER_LEVEL 20

#define MOB_PLAYER 1
#define MOB_ENEMY  2
#define MOB_DRAGON 3
#define MOB_MIND_FLAYER 4
#define MOB_DEMON 5

#define MOB_FORM_BIPED 1
#define MOB_FORM_QUADRAPED 2
#define MOB_FORM_FLYING 4

#include "item.h"
#include "lib/path.h"

typedef struct {
    int exp;
    int expNext;
    int level;
} PlayerAttributes;

typedef struct {
    Item *weapon;
    Item *armor;
} Equipment;

typedef struct {
    int hp, maxHP;
    rl_coords coords;
    int minDamage, maxDamage;
    int type;
    int form;
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
Mob *create_mob(int depth, rl_coords coords);

// try to attack x, y
// if no mob found at x, y do nothing
// return damage
int attack(Mob *attacker, Mob *target);

// insert mob into mobs list
int insert_mob(Mob *mob, Mob **mobs);

// insert into killed mobs array
int kill_mob(Mob *mob, Mobs *mobs);

// allocate mobs list
Mobs initialize_mobs();

// return mob name for symbol
const char* mob_name(char symbol);

#endif
