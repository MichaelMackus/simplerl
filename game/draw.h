#ifndef DRAW_H
#define DRAW_H

#include "dungeon.h"

// initialize our screen
// returns 0 on success
int init();

// end curses mode
void deinit();

// update & refresh the screen
void render(const Dungeon *dungeon);

// print list of mobs (i.e. "2 Dragons", "3 Kobolds"...)
void print_mob_list(Mobs mobs);

#endif
