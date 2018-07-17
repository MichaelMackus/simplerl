#ifndef GAME_H
#define GAME_H

#define GAME_PLAYING 0
#define GAME_WON 1
#define GAME_QUIT 2
#define GAME_DEATH 3
#define GAME_OOM 4
#define GAME_ERROR 5

#include "dungeon.h"

// initialize random dungeon
int init_level(Level *level, Mob *player);

// return GAME constant
int gameloop(Dungeon *dungeon, char input);

// does thing like autorest, and TODO automove
// return 1 if we should grab input this turn
int handle_input(Dungeon *dungeon);

#endif
