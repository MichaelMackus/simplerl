#ifndef RL_RNG_H
#define RL_RNG_H

typedef unsigned long (*rl_generator_f)(unsigned long min, unsigned long max);

// simple RNG implementation using C standard library
// if seed is 0 this seeds with system time
// NOTE: you probably want to use something else for best results on all systems
void rl_rng_stdlib_seed(unsigned int seed);
unsigned long rl_rng_stdlib_generate(unsigned long min, unsigned long max);

#ifdef RL_TWISTER
// RNG implementation using mersenne twister
// uses https://github.com/dajobe/libmtwist

// allocate mtwister buffer
void rl_rng_twister_create();
// free mtwister buffer
void rl_rng_twister_free();

// seed with user-defined seed (if 0 seeds with mtwist_seed_from_system)
void rl_rng_twister_seed(unsigned long seed);

unsigned long rl_rng_twister_generate(unsigned long min, unsigned long max);
#endif

#endif
