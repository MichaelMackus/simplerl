#include "rng.h"

#include <time.h>
#include <stdlib.h>

void rl_rng_stdlib_init()
{
    srand(time(0));
}

int rl_rng_stdlib_generate(int min, int max)
{
    // TODO assert/unsigned?
    if (min < 0 || max < 0 || (min == 0 && max == 0))
        return 0;

    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}
