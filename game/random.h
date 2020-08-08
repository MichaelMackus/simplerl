#ifndef RANDOM_H
#define RANDOM_H

// allocate our (P)RNG memory
void init_random();

// seed our (P)RNG
void seed_random();

// generate value >= min and <= max
int generate(int min, int max);

#endif
