#ifndef RL_MAPGEN_H
#define RL_MAPGEN_H

#include "bsp.h"
#include "rng.h"
#include "map.h"

/**
 * root is the root node to split from
 * generator is your RNG generator function
 * min_width defines the minimum width to split
 * min_height defines the minimum height to split
 * deviation is a a float value from 0.0-1.0 of how much the split can
 *     deviate from center (1 means the split can appear anywhere within
 *     the cell, and 0 means the split will always occur in the center)
 * max_recursion limits the split recursion
 */
void rl_recursively_split_bsp(rl_bsp *root, rl_generator_f generator,
        unsigned int min_width, unsigned int min_height,
        float deviation, int max_recursion);

/**
 * Create a map from a BSP tree. This adds rooms of random size into
 * each BSP leaf, and then connects every BSP sibling with cooridors
 * (traversing up the tree until all nodes are connected).
 */
rl_map *rl_create_map_from_bsp(rl_bsp *root, rl_generator_f generator,
        unsigned int room_min_width, unsigned int room_min_height,
        unsigned int room_max_width, unsigned int room_max_height,
        unsigned int room_padding, unsigned int max_adjacent_doors);

#endif
