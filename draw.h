#ifndef DRAW_H
#define DRAW_H

#include "map.h"

// initialize our screen
// returns 0 on success
int init();

// end curses mode
void deinit();

// update & refresh the screen
void update(const Dungeon *dungeon, const char **messages);

#endif
