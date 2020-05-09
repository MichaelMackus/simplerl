#include "bsp.h"

#include <stdlib.h>

struct rl_bsp_t {
    unsigned int width;
    unsigned int height;
    rl_coords_t loc;

    struct rl_bsp_t *left;
    struct rl_bsp_t *right;
};

rl_bsp_t *rl_create_bsp(unsigned int width, unsigned int height)
{
    rl_bsp_t *bsp = calloc(1, sizeof(rl_bsp_t));
    if (bsp == NULL)
        return NULL;

    bsp->width = width;
    bsp->height = height;

    return bsp;
}
void rl_split_bsp(rl_bsp_t *node, unsigned int position, rl_Split_Dir direction)
{
    // TODO assert not already split
    if (direction == RL_SPLIT_VERTICALLY && position >= node->height)
        return;
    if (direction == RL_SPLIT_HORIZONTALLY && position >= node->width)
        return;

    rl_bsp_t *left = malloc(sizeof(rl_bsp_t));
    if (left == NULL)
        return;
    rl_bsp_t *right = malloc(sizeof(rl_bsp_t));
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
rl_bsp_t *rl_get_bsp_left(rl_bsp_t *node)
{
    if (node == NULL) return NULL;
    return node->left;
}
rl_bsp_t *rl_get_bsp_right(rl_bsp_t *node)
{
    if (node == NULL) return NULL;
    return node->right;
}
unsigned int rl_get_bsp_width(rl_bsp_t *node)
{
    if (node == NULL) return 0;
    return node->width;
}
unsigned int rl_get_bsp_height(rl_bsp_t *node)
{
    if (node == NULL) return 0;
    return node->height;
}
rl_coords_t rl_get_bsp_loc(rl_bsp_t *node)
{
    if (node == NULL) return (rl_coords_t) {
        0
    };
    return node->loc;
}
int rl_is_bsp_leaf(rl_bsp_t *node)
{
    if (node == NULL) return 0;
    return (node->left == NULL && node->right == NULL);
}
