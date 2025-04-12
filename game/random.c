#include "random.h"
#include "lib/roguelike.h"

#include <stdlib.h>
#include <time.h>

void init_random(unsigned int seed)
{
    srand(seed);
}

int generate(int min, int max)
{
    return rl_rng_generate(min, max);
}
