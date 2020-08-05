#ifndef RL_MAPGEN_H
#define RL_MAPGEN_H

#include "bsp.h"
#include "rng.h"
#include "map.h"

void rl_recursively_split_bsp(rl_bsp *root, rl_generator_f generator,
        unsigned int min_width, unsigned int min_height, int max_recursion);

#endif
