#ifndef DRAW_H
#define DRAW_H

#include "dungeon.h"
#include "lib/roguelike.h"

// initialize our screen
// returns 0 on success
int init();

// end curses mode
void deinit();

// update & refresh the screen
void render(const Dungeon *dungeon);

// print the killed mob list
void print_mob_list(RL_Heap *heap);

#endif
