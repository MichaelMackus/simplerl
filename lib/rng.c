#include "rng.h"

#include <assert.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>

void rl_rng_stdlib_init()
{
    srand(time(0));
}

unsigned long rl_rng_stdlib_generate(unsigned long min, unsigned long max)
{
    assert(max > min);
    assert(max < RAND_MAX);

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

unsigned long rl_rng_twister_generate(unsigned long min, unsigned long max)
{
    assert(max > min);
    assert(max < MTWIST_FULL_MASK);

    return min + mtwist_drand(rl_twister_state) * (max - min + 1);
}
#endif
