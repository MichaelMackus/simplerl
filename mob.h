#ifndef MOB_H
#define MOB_H

#define MOB_PLAYER 1
#define MOB_ENEMY  2
#define MOB_NPC    3

typedef struct {
    int x, y;
    int type;
    char symbol;
} Mob;

#endif
