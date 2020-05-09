#ifndef RL_BSP_H
#define RL_BSP_H

#include "map.h"

struct rl_bsp_t;
typedef struct rl_bsp_t rl_bsp_t;

typedef enum {
    RL_SPLIT_HORIZONTALLY=1, // split the BSP node on the x axis (splits width)
    RL_SPLIT_VERTICALLY=2    // split the BSP node on the y axis (splits height)
} rl_Split_Dir;

rl_bsp_t *rl_create_bsp(unsigned int width, unsigned int height);
void rl_split_bsp(rl_bsp_t *node, unsigned int position, rl_Split_Dir direction);

rl_bsp_t *rl_get_bsp_left(rl_bsp_t *node);
rl_bsp_t *rl_get_bsp_right(rl_bsp_t *node);
unsigned int rl_get_bsp_width(rl_bsp_t *node);
unsigned int rl_get_bsp_height(rl_bsp_t *node);
rl_coords_t rl_get_bsp_loc(rl_bsp_t *node);
int rl_is_bsp_leaf(rl_bsp_t *node);

#endif
