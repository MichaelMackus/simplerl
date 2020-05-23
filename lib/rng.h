#ifndef RL_RNG_H
#define RL_RNG_H

typedef unsigned long (*rl_generator_f)(unsigned long min, unsigned long max);

// simple RNG implementation using C standard library
// NOTE: you probably want to use something else for best results on all systems
void rl_rng_stdlib_init();
unsigned long rl_rng_stdlib_generate(unsigned long min, unsigned long max);

#ifdef RL_TWISTER
// RNG implementation using mersenne twister
// uses https://github.com/dajobe/libmtwist
void rl_rng_twister_init();
void rl_rng_twister_free();
unsigned long rl_rng_twister_generate(unsigned long min, unsigned long max);
#endif

#endif
