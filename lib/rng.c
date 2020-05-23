#include "rng.h"

#include <limits.h>
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

#ifdef RL_TWISTER
#include "rng/mtwist/mt.c"

mtwist *rl_twister_state;
void rl_rng_twister_init()
{
    rl_twister_state = mtwist_new();
    mtwist_seed_from_system(rl_twister_state);
}
void rl_rng_twister_free()
{
    if (rl_twister_state) mtwist_free(rl_twister_state);
}

int rl_rng_twister_generate(int min, int max)
{
    // TODO assert/unsigned?
    if (min < 0 || max < 0 || (min == 0 && max == 0))
        return 0;

    return min + (int)(mtwist_drand(rl_twister_state) * (max - min + 1));
}
#endif
