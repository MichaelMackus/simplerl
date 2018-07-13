#ifndef GAME_H
#define GAME_H

#define GAME_PLAYING 0
#define GAME_WON 1
#define GAME_QUIT 2
#define GAME_DEATH 3
#define ERROR_OOM 100

#include "map.h"

// return GAME or ERROR constant
int gameloop(Dungeon *dungeon, const char **messages);

#endif
