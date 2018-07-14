#include "random.h"
#include <stdlib.h>
#include <time.h>

void seed_random()
{
    // simply use time since epoch as seed for now
    srand(time(0));
}

Box *random_cell(Level *level);
void fill_cell(Box *cell, Level *level);
void randomly_fill_tiles(Level *level)
{
    int maxCells = (rand() % MAX_CELLS) + MIN_CELLS;
    int cellCount = 0;
    int i = 0;

    // randomly fill cells
    while (cellCount < maxCells && i < MAX_RANDOM_RECURSION)
    {
        Box *cell = random_cell(level);
        if (cell != NULL)
        {
            fill_cell(cell, level);
            free(cell);
            ++cellCount;
        }
        ++i;
    }

    // TODO prune touching cells
    // TODO randomly fill corridors
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

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
int possibleCellDimensions[15][2] = {
    { 5, 5 },
    { 5, 6 },
    { 5, 7 },
    { 5, 8 },
    { 6, 4 },
    { 6, 5 },
    { 6, 6 },
    { 6, 8 },
    { 7, 4 },
    { 7, 5 },
    { 7, 6 },
    { 7, 8 },
    { 8, 5 },
    { 8, 6 },
    { 8, 7 },
};

Dimensions random_dimensions();
Box *random_cell(Level *level)
{
    Coords coords;
    int impassable, i;
    Dimensions dimensions;

    do
    {
        coords = random_passable_coords(level);
        dimensions = random_dimensions();

        // check all tiles if passable from coords -> dimension
        impassable = 0;
        for (int y = coords.y; y < coords.y + dimensions.h; ++y)
        {
            for (int x = coords.x; x < coords.x + dimensions.w; ++x)
            {
                const Tile *t = get_tile(level, y, x);
                if (t == NULL || !is_passable(*t))
                {
                    ++impassable;
                    break;
                }
            }
            if (impassable > 0)
                break;
        }

        ++i;
    }
    while (impassable > 0 && i < MAX_RANDOM_RECURSION);

    if (impassable == 0)
    {
        Box *box;
        box = malloc(sizeof(Box));
        box->coords = coords;
        box->dimensions = dimensions;

        return box;
    }

    return NULL;
}

// fill cell with empty space, surrounding with walls
void fill_cell(Box *cell, Level *level)
{
    for (int y = cell->coords.y; y < cell->coords.y + cell->dimensions.h; ++y)
    {
        for (int x = cell->coords.x; x < cell->coords.x + cell->dimensions.w; ++x)
        {
            // fill with empty space surrounded by walls

            Tile t;
            if (y == cell->coords.y || y == cell->coords.y + cell->dimensions.h - 1)
                t = create_tile(TILE_WALL);
            else if (x == cell->coords.x || x == cell->coords.x + cell->dimensions.w - 1)
                t = create_tile(TILE_WALL_SIDE);
            else
                t = create_tile(TILE_FLOOR);

            level->tiles[y][x] = t;
        }
    }
}

Dimensions random_dimensions()
{
    int i = rand() % 15;

    Dimensions dimensions;
    dimensions.w = possibleCellDimensions[i][0];
    dimensions.h = possibleCellDimensions[i][1];

    return dimensions;
}
