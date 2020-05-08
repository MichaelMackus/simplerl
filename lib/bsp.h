#ifndef RL_BSP_H
#define RL_BSP_H

#include "map.h"

struct RL_bsp_t;
typedef struct RL_bsp_t RL_bsp_t;

typedef enum {
    RL_SPLIT_HORIZONTALLY=1, // split the BSP node on the x axis (splits width)
    RL_SPLIT_VERTICALLY=2    // split the BSP node on the y axis (splits height)
} RL_Split_Dir;

RL_bsp_t *rl_create_bsp(unsigned int width, unsigned int height);
void rl_split_bsp(RL_bsp_t *node, unsigned int position, RL_Split_Dir direction);

RL_bsp_t *rl_get_bsp_left(RL_bsp_t *node);
RL_bsp_t *rl_get_bsp_right(RL_bsp_t *node);
unsigned int rl_get_bsp_width(RL_bsp_t *node);
unsigned int rl_get_bsp_height(RL_bsp_t *node);
RL_coords_t rl_get_bsp_loc(RL_bsp_t *node);
int rl_is_bsp_leaf(RL_bsp_t *node);

#endif
