#include "bsp.h"

#include <stdlib.h>

struct RL_bsp_t {
    unsigned int width;
    unsigned int height;
    RL_coords_t loc;

    struct RL_bsp_t *left;
    struct RL_bsp_t *right;
};

RL_bsp_t *rl_create_bsp(unsigned int width, unsigned int height)
{
    RL_bsp_t *bsp = calloc(1, sizeof(RL_bsp_t));
    if (bsp == NULL)
        return NULL;

    bsp->width = width;
    bsp->height = height;

    return bsp;
}
void rl_split_bsp(RL_bsp_t *node, unsigned int position, RL_Split_Dir direction)
{
    // TODO assert not already split
    if (direction == RL_SPLIT_VERTICALLY && position >= node->height)
        return;
    if (direction == RL_SPLIT_HORIZONTALLY && position >= node->width)
        return;

    RL_bsp_t *left = malloc(sizeof(RL_bsp_t));
    if (left == NULL)
        return;
    RL_bsp_t *right = malloc(sizeof(RL_bsp_t));
    if (right == NULL) {
        free(left);
        return;
    }

    if (direction == RL_SPLIT_VERTICALLY) {
        left->width = node->width;
        left->height = position;
        left->loc = node->loc;
        right->width = node->width;
        right->height = node->height - position;
        right->loc = node->loc;
        right->loc.y += position;
    } else {
        left->width = position;
        left->height = node->height;
        left->loc = node->loc;
        right->width = node->width - position;
        right->height = node->height;
        right->loc = node->loc;
        right->loc.x += position;
    }

    node->left = left;
    node->right = right;
}

// TODO assert on NULL?
RL_bsp_t *rl_get_bsp_left(RL_bsp_t *node)
{
    if (node == NULL) return NULL;
    return node->left;
}
RL_bsp_t *rl_get_bsp_right(RL_bsp_t *node)
{
    if (node == NULL) return NULL;
    return node->right;
}
unsigned int rl_get_bsp_width(RL_bsp_t *node)
{
    if (node == NULL) return 0;
    return node->width;
}
unsigned int rl_get_bsp_height(RL_bsp_t *node)
{
    if (node == NULL) return 0;
    return node->height;
}
RL_coords_t rl_get_bsp_loc(RL_bsp_t *node)
{
    if (node == NULL) return (RL_coords_t) {
        0
    };
    return node->loc;
}
int rl_is_bsp_leaf(RL_bsp_t *node)
{
    if (node == NULL) return 0;
    return (node->left == NULL && node->right == NULL);
}
