#include "random.h"
#include "lib/rng.h"

#include <stdlib.h>
#include <time.h>

void seed_random()
{
    // simply use time since epoch as seed for now
    rl_rng_stdlib_init();
}

int generate(int min, int max)
{
    return rl_rng_stdlib_generate(min, max);
}
