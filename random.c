#include "random.h"
#include <stdlib.h>
#include <time.h>

void seed_random()
{
    // simply use time since epoch as seed for now
    srand(time(0));
}

int generate(int min, int max)
{
    int damage = rand() % (max-min+1)  +  min;
}
