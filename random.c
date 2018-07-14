#include "random.h"
#include <memory.h>
#include <stdlib.h>
#include <time.h>

void seed_random()
{
    // simply use time since epoch as seed for now
    srand(time(0));
}

Box *random_cell(Level *level);
void fill_cell(Box *cell, Level *level, int fill);
void randomly_fill_corridors(Level *level, const Box **cells, int startIndex, int cellCount);
int branches(const Box *cell, const Level *level);
void randomly_fill_tiles(Level *level)
{
    int maxCells = (rand() % MAX_CELLS) + MIN_CELLS;
    int cellCount = 0;
    int i = 0;

    // randomly fill cells
    Box **cells;
    cells = malloc(sizeof(Box*) * maxCells);
    while (cellCount < maxCells && i < MAX_RANDOM_RECURSION)
    {
        Box *cell = random_cell(level);
        if (cell != NULL)
        {
            fill_cell(cell, level, 1);
            cells[cellCount] = cell;
            ++cellCount;
        }
        ++i;
    }

    // fill corridors between cells
    randomly_fill_corridors(level, (const Box**) cells, 0, cellCount);

    // free cells & prune non-connecting cells
    for (i = 0; i < cellCount; ++i)
    {
        if (branches(cells[i], level) == 0)
            fill_cell(cells[i], level, 0);
        free(cells[i]);
    }
    free(cells);
}

int is_neighbor(const Box *start, const Box *target, const Box **cells, int cellCount);
void draw_line(const Box *start, const Box *target, Level *level);
void randomly_fill_corridors(Level *level, const Box **cells, int startIndex, int cellCount)
{
    /**
     *
     * RANDOM CORRIDOR GENERATION PSEUDO-CODE
     *
     * neighbor:
     *  cell where the a* line from centers does not pass through
     *  another cell
     * branch:
     *  space in wall of cell, connecting to another cell
     *
     * LOOP:
     *  Pick cell with no branches
     *  Pick neighbor (with at most 1 branch) at random
     *  Draw corridor to neighbor
     *  Start loop over with neighbor as cell
     *
     * DONE:
     *  Remove non-connected cells
     *
     */

    // can't fill corridor if no start cell
    if (startIndex >= cellCount || cells[startIndex] == NULL)
        return;

    // limit array to possible neighbors
    const Box **neighbors;
    neighbors = malloc(sizeof(Box*) * cellCount);
    memset(neighbors, 0, sizeof(Box*) * cellCount);
    int maxNeighbors = 0;
    for (int i = startIndex + 1; i < cellCount; ++i) // check cells other than start cell
    {
        // check for clear line from center of start to center of neighbor
        // TODO we should probably do this from a wall, and pick the wall here
        if (is_neighbor(cells[startIndex], cells[i], cells, cellCount))
        {
            // neighbor can have at most 1 branch
            if (branches(cells[i], level) <= 1)
            {
                neighbors[maxNeighbors] = cells[i];
                ++maxNeighbors;
            }
        }
    }

    if (maxNeighbors > 0)
    {
        // pick a random neighbor with at most 1 branch
        int neighborIndex = rand() % maxNeighbors;

        // draw line to neighbor
        draw_line(cells[startIndex], cells[neighborIndex], level);
    }

    free(neighbors);

    return randomly_fill_corridors(level, cells, startIndex + 1, cellCount);
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

// draws a line using straight lines (just a simple right angle for now)
const Coords find_wall_for_branch(const Box *cell, const Level *level);
void draw_line(const Box *start, const Box *target, Level *level)
{
    const Coords a = find_wall_for_branch(start, (const Level*) level);
    const Coords b = find_wall_for_branch(target, (const Level*) level);

    // handle NULL case (i.e. there was some error in find_wall_for_branch)
    if (a.x == b.x && a.y == b.y)
        return;

    // make cavern in start tile
    level->tiles[a.y][a.x].type = TILE_CAVERN;

    // x & y direction
    int dy = 0, dx = 0;

    // pick direction based on which wall
    if (a.y == start->coords.y)
        // direction *up*
        dy = -1;
    else if (a.y == start->coords.y + start->dimensions.h - 1)
        // direction *down*
        dy = 1;
    else if (a.x == start->coords.x)
        // direction *left*
        dx = -1;
    else if (a.x == start->coords.x + start->dimensions.w - 1)
        // direction *right*
        dx = 1;

    // draw caverns in direction of b
    Coords current;
    current.x = a.x;
    current.y = a.y;
    while (current.x != b.x || current.y != b.y)
    {
        current.y += dy;
        current.x += dx;

        if (level->tiles[current.y][current.x].type != TILE_FLOOR)
            level->tiles[current.y][current.x].type = TILE_CAVERN;

        // switch direction based on current spot
        if (dy < 0 && current.y + dy < b.y)
        {
            dy = 0;
            if (current.x < b.x)
                dx = 1;
            else if (current.x > b.x)
                dx = -1;
            else
                dy = 1;
        }
        else if (dy > 0 && current.y + dy > b.y)
        {
            dy = 0;
            if (current.x < b.x)
                dx = 1;
            else if (current.x > b.x)
                dx = -1;
            else
                dy = -1;
        }
        else if (dx < 0 && current.x + dx < b.x)
        {
            dx = 0;
            if (current.y < b.y)
                dy = 1;
            else if (current.y > b.y)
                dy = -1;
            else
                dx = 1;
        }
        else if (dx > 0 && current.x + dx > b.x)
        {
            dx = 0;
            if (current.y < b.y)
                dy = 1;
            else if (current.y > b.y)
                dy = -1;
            else
                dx = -1;
        }
    }
}

Coords random_coords(Level *level)
{
    Coords coords;
    coords.x = rand() % MAX_WIDTH;
    coords.y = rand() % MAX_HEIGHT;

    return coords;
}

Coords random_open_coords(Level *level)
{
    // do simple brute force attempt to get an open coord
    while (1)
    {
        Coords coords = random_coords(level);
        const Tile *t = get_tile(level, coords.y, coords.x);
        if (t != NULL && t->type == TILE_NONE)
            return coords;
    }
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
        coords = random_open_coords(level);
        dimensions = random_dimensions();

        // check all tiles if passable from coords -> dimension
        impassable = 0;
        for (int y = coords.y; y < coords.y + dimensions.h; ++y)
        {
            for (int x = coords.x; x < coords.x + dimensions.w; ++x)
            {
                const Tile *t = get_tile(level, y, x);
                if (t == NULL || t->type != TILE_NONE)
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
// flip fill flag to 0 to blank the cell
void fill_cell(Box *cell, Level *level, int fill)
{
    for (int y = cell->coords.y; y < cell->coords.y + cell->dimensions.h; ++y)
    {
        for (int x = cell->coords.x; x < cell->coords.x + cell->dimensions.w; ++x)
        {
            // fill with empty space surrounded by walls

            Tile t;
            if (fill)
                if (y == cell->coords.y || y == cell->coords.y + cell->dimensions.h - 1)
                    t = create_tile(TILE_WALL);
                else if (x == cell->coords.x || x == cell->coords.x + cell->dimensions.w - 1)
                    t = create_tile(TILE_WALL_SIDE);
                else
                    t = create_tile(TILE_FLOOR);
            else
                t = create_tile(TILE_NONE);

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

// checks for a clear straight line from center of start to center of target
// return 1 if possible neighbor
int within_cell(const Coords c, const Box cell);
const Coords **get_line(const Coords a, const Coords b);
void free_line(const Coords **line);
int is_neighbor(const Box *start, const Box *target, const Box **cells, int cellCount)
{
    Coords startCenter;
    startCenter.x = start->coords.x + ((start->dimensions.w - start->coords.x) / 2);
    startCenter.y = start->coords.y + ((start->dimensions.h - start->coords.y) / 2);

    Coords targetCenter = target->coords;
    targetCenter.x = target->coords.x + ((target->dimensions.w - target->coords.x) / 2);
    targetCenter.y = target->coords.y + ((target->dimensions.h - target->coords.y) / 2);

    // draw straight line from start -> target
    const Coords **line = get_line(startCenter, targetCenter);

    // check for any coord that passes through another cell
    int i = 0;
    while (line[i] != NULL)
    {
        // ensure we're not comparing start or target
        if (!within_cell(*line[i], *start) && !within_cell(*line[i], *target))
        {
            // check within cells for intersection *not* start & *not* target
            for (int j = 0; j < cellCount; ++j)
            {
                const Box *cell = cells[j];

                // ensure we're not comparing start or target cell
                if (within_cell(cell->coords, *start) || within_cell(cell->coords, *target))
                    continue;

                // if we're within *another* cell, this is no neighbor
                if (within_cell(*line[i], *cell))
                {
                    free_line(line);

                    return 0;
                }
            }
        }
        ++i;
    }

    free_line(line);

    return 1;
}

// return 1 if coords are within cell
int within_cell(const Coords c, const Box cell)
{
    for (int y = cell.coords.y; y < cell.coords.y + cell.dimensions.h; ++y)
        for (int x = cell.coords.x; x < cell.coords.x + cell.dimensions.w; ++x)
            if (c.x == x && c.y == y)
                return 1;

    return 0;
}

// calculate amount of branches (i.e. holes in cell walls)
int branches(const Box *cell, const Level *level)
{
    // loop through walls of cell and check for tiles that are not walls
    int count = 0;
    for (int y = cell->coords.y; y < cell->coords.y + cell->dimensions.h; ++y)
        for (int x = cell->coords.x; x < cell->coords.x + cell->dimensions.w; ++x)
        {
            if ((y == cell->coords.y || y == cell->coords.y + cell->dimensions.h - 1) ||
                    (x == cell->coords.x || x == cell->coords.x + cell->dimensions.w - 1))
            {
                // increment branches count if this is *not* a wall
                const Tile *t = get_tile(level, y, x);
                if (t != NULL && t->type != TILE_NONE && t->type != TILE_WALL && t->type != TILE_WALL_SIDE)
                    ++count;
            }
        }

    return count;
}

// get straight line from a to b
const Coords **get_line(const Coords a, const Coords b)
{
    Coords **line;
    line = malloc(sizeof(Coords*)*MAX_WIDTH*MAX_HEIGHT);
    memset(line, 0, sizeof(Coords*)*MAX_WIDTH*MAX_HEIGHT);

    Coords currentCoords = a;
    for (int i = 0; i < MAX_WIDTH + MAX_HEIGHT; ++i)
    {
        line[i] = malloc(sizeof(Coords));

        // calculate x difference
        if (b.x - currentCoords.x > 0)
            ++currentCoords.x;
        else if (b.x - currentCoords.x < 0)
            --currentCoords.x;
        // calculate y difference
        if (b.y - currentCoords.y > 0)
            ++currentCoords.y;
        else if (b.y - currentCoords.y < 0)
            --currentCoords.y;

        line[i]->x = currentCoords.x;
        line[i]->y = currentCoords.y;

        // we're done if we are at the end
        if (currentCoords.x == b.x && currentCoords.y == b.y)
            break;
    }

    return (const Coords **) line;
}

void free_line(const Coords **line)
{
    int i = 0;
    while (line[i] != NULL)
    {
        free((Coords*) line[i]);
        ++i;
    }
    free((Coords**) line);
}

const Coords find_wall_for_branch(const Box *cell, const Level *level)
{
    // find an opening in cell for a branch
    int ycount = 0, xcount = 0;
    for (int y = cell->coords.y; y < cell->coords.y + cell->dimensions.h; ++y)
    {
        for (int x = cell->coords.x; x < cell->coords.x + cell->dimensions.w; ++x)
        {
            // check start & end rows, if walls increment xcount
            if (y == cell->coords.y || y == cell->coords.y + cell->dimensions.h - 1)
            {
                // TODO check wall closest to neighbor *first*

                // guard against being next to world edge
                if (y > 0 && y < MAX_HEIGHT - 1)
                {
                    if (x == cell->coords.x)
                        xcount = 0;

                    // increment x count if it is a wall
                    const Tile *t = get_tile(level, y, x);
                    if (t != NULL && (t->type == TILE_WALL || t->type == TILE_WALL_SIDE))
                        ++xcount;

                    if (xcount == cell->dimensions.w)
                    {
                        Coords c;
                        c.x = cell->coords.x + (cell->dimensions.w / 2);
                        c.y = y;

                        return c;
                    }
                }
            }

            // check start & end columns, if walls increment ycount
            if (x == cell->coords.x || x == cell->coords.x + cell->dimensions.w - 1)
            {
                // guard against being next to world edge
                if (x > 0 && x < MAX_WIDTH - 1)
                {
                    if (y == cell->coords.y)
                        ycount = 0;

                    // increment y count if it is a wall
                    const Tile *t = get_tile(level, y, x);
                    if (t != NULL && (t->type == TILE_WALL || t->type == TILE_WALL_SIDE))
                        ++ycount;

                    if (ycount == cell->dimensions.h)
                    {
                        Coords c;
                        c.x = x;
                        c.y = cell->coords.y + (cell->dimensions.h / 2);

                        return c;
                    }
                }
            }
        }
    }
}
