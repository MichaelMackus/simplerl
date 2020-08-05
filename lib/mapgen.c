#include "mapgen.h"

#include <assert.h>

#define MAX_GENERATOR_RECURSION 10

void rl_recursively_split_bsp(rl_bsp *root, rl_generator_f generator,
        unsigned int min_width, unsigned int min_height,
        unsigned int max_width, unsigned int max_height,
        float deviation, int max_recursion)
{
    if (max_recursion <= 0)
        return;

    assert(deviation >= 0.0 && deviation <= 1.0);

    unsigned int width = rl_get_bsp_width(root);
    unsigned int height = rl_get_bsp_height(root);
    /* unsigned int max_width = width - min_width; */
    /* unsigned int max_height = height - min_height; */

    // determine split dir & split
    rl_split_dir dir;
    if (width > max_width && height <= max_height) {
        dir = RL_SPLIT_HORIZONTALLY;
    } else if (height > max_height && width <= max_width) {
        dir = RL_SPLIT_VERTICALLY;
    } else if (generator(0, 1)) {
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

    unsigned int split_position;
    if (dir == RL_SPLIT_HORIZONTALLY) {
        // cannot split if current node size is too small
        if (width < min_width*2)
            return;

        unsigned int center = width / 2;
        unsigned int from = center - (center * deviation);
        unsigned int to = center + (center * deviation);
        if (from < min_width)
            from = min_width;
        if (to > width - min_width)
            to = width - min_width;

        split_position = generator(from, to);
    } else {
        // cannot split if current node size is too small
        if (height < min_height*2)
            return;

        unsigned int center = height / 2;
        unsigned int from = center - (center * deviation);
        unsigned int to = center + (center * deviation);
        if (from < min_height)
            from = min_height;
        if (to > height - min_height)
            to = height - min_height;

        split_position = generator(from, to);
    }

    rl_split_bsp(root, split_position, dir);

    // continue recursion
    rl_bsp *left = rl_get_bsp_left(root);
    rl_bsp *right = rl_get_bsp_right(root);

    if (left == NULL || right == NULL)
        return;

    rl_recursively_split_bsp(left, generator,
            min_width, min_height,
            max_width, max_height,
            deviation, max_recursion - 1);
    rl_recursively_split_bsp(right, generator,
            min_width, min_height,
            max_width, max_height,
            deviation, max_recursion - 1);
}
