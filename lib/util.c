#include "util.h"

#include <math.h>

double manhattan_distance(rl_coords node, rl_coords end)
{
    return abs(node.x - end.x) + abs(node.y - end.y);
}

double euclidian_distance(rl_coords node, rl_coords end)
{
    return sqrt(pow(node.x - end.x, 2) + pow(node.y - end.y, 2));
}
