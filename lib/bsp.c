#include "bsp.h"

#include <assert.h>
#include <stdlib.h>
#include <limits.h>

struct rl_bsp {
    unsigned int width;
    unsigned int height;
    rl_coords loc;

    struct rl_bsp *parent;
    struct rl_bsp *left;
    struct rl_bsp *right;
};

rl_bsp *rl_create_bsp(unsigned int width, unsigned int height)
{
    rl_bsp *bsp = calloc(1, sizeof(rl_bsp));
    if (bsp == NULL)
        return NULL;

    bsp->width = width;
    bsp->height = height;
    bsp->parent = NULL;

    return bsp;
}
void rl_split_bsp(rl_bsp *node, unsigned int position, rl_split_dir direction)
{
    // can't split something already split
    assert(node->left == NULL && node->right == NULL);

    if (direction == RL_SPLIT_VERTICALLY && position >= node->height)
        return;
    if (direction == RL_SPLIT_HORIZONTALLY && position >= node->width)
        return;

    rl_bsp *left = malloc(sizeof(rl_bsp));
    if (left == NULL)
        return;
    rl_bsp *right = malloc(sizeof(rl_bsp));
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

    left->parent = right->parent = node;
    node->left = left;
    node->right = right;
}

// TODO assert on NULL?
rl_bsp *rl_get_bsp_left(rl_bsp *node)
{
    if (node == NULL) return NULL;
    return node->left;
}
rl_bsp *rl_get_bsp_right(rl_bsp *node)
{
    if (node == NULL) return NULL;
    return node->right;
}
rl_bsp *rl_get_bsp_parent(rl_bsp *node)
{
    if (node == NULL) return NULL;
    return node->parent;
}
rl_bsp *rl_get_bsp_sibling(rl_bsp *node)
{
    if (node == NULL) return NULL;
    if (node->parent == NULL) return NULL;

    if (node == node->parent->left) {
        return node->parent->right;
    } else if (node == node->parent->right) {
        return node->parent->left;
    }

    return NULL;
}
unsigned int rl_get_bsp_width(rl_bsp *node)
{
    if (node == NULL) return 0;
    return node->width;
}
unsigned int rl_get_bsp_height(rl_bsp *node)
{
    if (node == NULL) return 0;
    return node->height;
}
rl_coords rl_get_bsp_loc(rl_bsp *node)
{
    if (node == NULL) return (rl_coords) {
        0
    };
    return node->loc;
}
int rl_get_bsp_depth(rl_bsp *node)
{
    int depth = 0;
    while (node && node->parent) {
        node = node->parent;
        depth++;
    }

    return depth;
}
int rl_is_bsp_leaf(rl_bsp *node)
{
    if (node == NULL) return 0;
    return (node->left == NULL && node->right == NULL);
}

int rl_is_bsp_left(rl_bsp *node)
{
    if (node == NULL) return 0;
    if (node->parent == NULL) return 0;
    
    return node->parent->left == node;
}

int rl_is_bsp_right(rl_bsp *node)
{
    if (node == NULL) return 0;
    if (node->parent == NULL) return 0;

    return node->parent->right == node;
}

void push_leaves(rl_queue **queue, rl_bsp *node)
{
    if (node->left) {
        if (rl_is_bsp_leaf(node->left)) {
            rl_push(queue, node->left, INT_MAX - rl_get_bsp_depth(node->left));
        } else {
            push_leaves(queue, node->left);
        }
    }

    if (node->right) {
        if (rl_is_bsp_leaf(node->right)) {
            rl_push(queue, node->right, INT_MAX - rl_get_bsp_depth(node->right));
        } else {
            push_leaves(queue, node->right);
        }
    }
}
rl_queue *rl_get_bsp_leaves(rl_bsp *node)
{
    rl_queue *queue = NULL;
    push_leaves(&queue, node);

    return queue;
}

void push_nodes(rl_queue **queue, rl_bsp *node)
{
    if (node == NULL) return;

    // TODO FIXME
    rl_push(queue, node, INT_MAX - rl_get_bsp_depth(node));

    if (node->left) {
        push_nodes(queue, node->left);
    }

    if (node->right) {
        push_nodes(queue, node->right);
    }
}
rl_queue *rl_get_bsp_nodes(rl_bsp *node)
{
    rl_queue *queue = NULL;
    push_nodes(&queue, node);

    return queue;
}
