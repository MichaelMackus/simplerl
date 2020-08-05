#include "mapgen.h"

#define MAX_GENERATOR_RECURSION 10

void rl_recursively_split_bsp(rl_bsp *root, rl_generator_f generator,
        unsigned int min_width, unsigned int min_height, int max_recursion)
{
    if (max_recursion <= 0)
        return;

    unsigned int width = rl_get_bsp_width(root);
    unsigned int height = rl_get_bsp_height(root);

    unsigned int max_width = width - min_width;
    unsigned int max_height = height - min_height;

    // determine split dir & split
    rl_split_dir dir;
    if (generator(0, 1)) {
        if (width < min_width*2)
            dir = RL_SPLIT_VERTICALLY;
        else
            dir = RL_SPLIT_HORIZONTALLY;
    } else {
        if (height < min_height*2)
            dir = RL_SPLIT_HORIZONTALLY;
        else
            dir = RL_SPLIT_VERTICALLY;
    }

    if (dir == RL_SPLIT_HORIZONTALLY) {
        // cannot split if current node size is too small
        if (width < min_width*2)
            return;

        int split_width = generator(min_width, max_width);
        rl_split_bsp(root, split_width, RL_SPLIT_HORIZONTALLY);
    } else {
        // cannot split if current node size is too small
        if (height < min_height*2)
            return;

        int split_height = generator(min_height, max_height);
        rl_split_bsp(root, split_height, RL_SPLIT_VERTICALLY);
    }

    // continue recursion
    rl_bsp *left = rl_get_bsp_left(root);
    rl_bsp *right = rl_get_bsp_right(root);

    if (left == NULL || right == NULL)
        return;

    rl_recursively_split_bsp(left, generator,
            min_width, min_height, max_recursion - 1);
    rl_recursively_split_bsp(right, generator,
            min_width, min_height, max_recursion - 1);
}
