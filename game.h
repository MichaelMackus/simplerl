#ifndef GAME_H
#define GAME_H

#define GAME_PLAYING 0
#define GAME_WON 1
#define GAME_QUIT 2
#define GAME_DEATH 3
#define GAME_OOM 4

#include "map.h"
#include <stdint.h>

// initialize dungeon defaults
int init_game(Dungeon *dungeon, const char **messages);

// return GAME constant
int gameloop(Dungeon *dungeon, const char **messages);

#endif
