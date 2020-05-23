#include "random.h"
#include "lib/rng.h"

#include <stdlib.h>
#include <time.h>

void seed_random()
{
    rl_rng_twister_init();
}

int generate(int min, int max)
{
    return rl_rng_twister_generate(min, max);
}
