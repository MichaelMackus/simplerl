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
 * each BSP leaf.
 *
 * You will need to connect the corridors afterwards - you can call one
 * of the rl_connect_corridors_* functions or do this manually.
 */
rl_map *rl_create_map_from_bsp(rl_bsp *root, rl_generator_f generator,
        unsigned int room_min_width, unsigned int room_min_height,
        unsigned int room_max_width, unsigned int room_max_height,
        unsigned int room_padding);

// tries to connect corridors to random passable tiles in siblings
// max_adjacent_doors - defines max adjacent doorways for post
//     processing; set to 0 for unlimited adjacent doors
void rl_connect_corridors_to_random_siblings(rl_map *map, rl_bsp *root, rl_generator_f generator, unsigned int max_adjacent_doors);
// connects corridors to closest sibling rooms
void rl_connect_corridors_to_closest_siblings(rl_map *map, rl_bsp *root, rl_generator_f generator);

#endif
