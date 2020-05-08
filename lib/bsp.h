#ifndef BSP_H
#define BSP_H

#include "map.h"

struct _RL_BSP;
typedef struct _RL_BSP RL_BSP;

typedef enum {
    RL_SPLIT_HORIZONTALLY=1, // split the BSP node on the x axis (splits width)
    RL_SPLIT_VERTICALLY=2    // split the BSP node on the y axis (splits height)
} RL_Split_Dir;

RL_BSP* rl_create_bsp(unsigned int width, unsigned int height);
void rl_split_bsp(RL_BSP* node, unsigned int position, RL_Split_Dir direction);

RL_BSP* rl_get_bsp_left(RL_BSP* node);
RL_BSP* rl_get_bsp_right(RL_BSP* node);
unsigned int rl_get_bsp_width(RL_BSP* node);
unsigned int rl_get_bsp_height(RL_BSP* node);
RL_Coords rl_get_bsp_loc(RL_BSP* node);
int rl_is_bsp_leaf(RL_BSP* node);

#endif
