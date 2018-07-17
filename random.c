#include "random.h"
#include <stdlib.h>
#include <time.h>

void seed_random()
{
    // simply use time since epoch as seed for now
    srand(time(0));
}
