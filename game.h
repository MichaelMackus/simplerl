#ifndef GAME_H
#define GAME_H

#define GAME_PLAYING 0
#define GAME_WON 1
#define GAME_QUIT 2
#define GAME_DEATH 3
// errors are divisible by 4
#define ERROR_OOM 4

#include "map.h"
#include <stdint.h>

// initialize dungeon defaults
int init_game(Dungeon *dungeon, const char **messages);

// return GAME or ERROR constant
// divide by 4 (or bitshift twice) to get program error code
uint8_t gameloop(Dungeon *dungeon, const char **messages);

#endif
