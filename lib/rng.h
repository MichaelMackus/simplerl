#ifndef RL_RNG_H
#define RL_RNG_H

typedef int (*rl_generator_f)(int min, int max);

// simple RNG implementation using C standard library
// NOTE: you probably want to use something else for best results
void rl_rng_stdlib_init();
int rl_rng_stdlib_generate(int min, int max);

#endif
