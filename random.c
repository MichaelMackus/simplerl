#include "random.h"
#include "lib/rng.h"

#include <stdlib.h>
#include <time.h>

void init_random()
{
    rl_rng_twister_create();
}

void seed_random()
{
    rl_rng_twister_seed(0);
}

int generate(int min, int max)
{
    return rl_rng_twister_generate(min, max);
}
