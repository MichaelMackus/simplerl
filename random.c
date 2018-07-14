#include "random.h"
#include <stdlib.h>
#include <time.h>

void seed_random()
{
    // simply use time since epoch as seed for now
    srand(time(0));
}

Coords random_coords(Level *level)
{
    Coords coords;
    coords.x = rand() % MAX_WIDTH;
    coords.y = rand() % MAX_HEIGHT;

    return coords;
}

Coords random_passable_coords(Level *level)
{
    // do simple brute force attempt to get a passable coord
    while (1)
    {
        Coords coords = random_coords(level);
        const Tile *t = get_tile(level, coords.y, coords.x);
        if (t != NULL && is_passable(*t))
            return coords;
    }
}

// list of possible cell dimensions (w x h)
int possibleCellDimensions[][2] = {
    { 2, 3 },
    { 2, 4 },
    { 2, 5 },
    { 2, 6 },
    { 3, 3 },
    { 3, 4 },
    { 3, 5 },
    { 4, 2 },
    { 4, 3 },
    { 4, 4 },
    { 4, 5 },
    { 4, 6 },
    { 5, 4 },
    { 5, 5 },
    { 5, 6 },
};

const int *random_dimension();
Box random_cell(Level *level, int w, int h)
{
    Coords coords;
    int impassable;
    const int *dimension;

    do
    {
        coords = random_coords(level);
        dimension = random_dimension();

        // check all tiles if passable from coords -> dimension
        impassable = 0;
        for (int x = coords.x; x < coords.x + dimension[0]; ++x)
        {
            for (int y = coords.y; y < coords.y + dimension[1]; ++y)
            {
                const Tile *t = get_tile(level, y, x);
                if (t == NULL || !is_passable(*t))
                {
                    ++impassable;
                }
            }
        }
    }
    while (impassable > 0);

    Box box;
    box.coords = coords;
    box.w = dimension[0];
    box.h = dimension[1];

    return box;
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

const int *random_dimension()
{
    int i = rand() % 15;

    return possibleCellDimensions[i];
}

