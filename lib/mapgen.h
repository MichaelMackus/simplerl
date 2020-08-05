#ifndef RL_MAPGEN_H
#define RL_MAPGEN_H

#include "bsp.h"
#include "rng.h"
#include "map.h"

/**
 * Recursively split a BSP tree until max_recursion is hit. Note that
 * the "max_width" and "max_height" are not hard rules unless
 * max_recursion is set sufficiently high.
 *
 * root is the root node to split from
 * generator is your RNG generator function
 * min_width defines the minimum width to split
 * min_height defines the minimum height to split
 * max_width defines the suggested maximum width to split
 * max_height defines the suggested maximum height to split
 * deviation is a a float value from 0.0-1.0 of how much the split can
 *     deviate from center (1 means the split can appear anywhere within
 *     the cell, and 0 means the split will always occur in the center)
 * max_recursion limits the split recursion
 */
void rl_recursively_split_bsp(rl_bsp *root, rl_generator_f generator,
        unsigned int min_width, unsigned int min_height,
        unsigned int max_width, unsigned int max_height,
        float deviation, int max_recursion);

#endif
