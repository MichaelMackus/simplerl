#ifndef RL_BSP_H
#define RL_BSP_H

#include "map.h"
#include "queue.h"

struct rl_bsp;
typedef struct rl_bsp rl_bsp;

typedef enum {
    RL_SPLIT_HORIZONTALLY=1, // split the BSP node on the x axis (splits width)
    RL_SPLIT_VERTICALLY=2    // split the BSP node on the y axis (splits height)
} rl_split_dir;

rl_bsp *rl_create_bsp(unsigned int width, unsigned int height);
void rl_split_bsp(rl_bsp *node, unsigned int position, rl_split_dir direction);

rl_bsp *rl_get_bsp_left(rl_bsp *node);
rl_bsp *rl_get_bsp_right(rl_bsp *node);
unsigned int rl_get_bsp_width(rl_bsp *node);
unsigned int rl_get_bsp_height(rl_bsp *node);
rl_coords rl_get_bsp_loc(rl_bsp *node);
int rl_is_bsp_leaf(rl_bsp *node);
rl_queue *rl_get_bsp_leaves(rl_bsp *node);

#endif
