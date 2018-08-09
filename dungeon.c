#include "dungeon.h"
#include <stdlib.h>
#include <memory.h>

Dungeon *create_dungeon()
{
    // do this otherwise initial seed will always be the same
    seed_random();

    // allocate dungeon
    Dungeon *dungeon;
    dungeon = malloc(sizeof(Dungeon));

    // handle out of memory case
    if (dungeon == NULL)
        return NULL;

    dungeon->turn = 0;

    // allocate killed list
    dungeon->killed = initialize_mobs();

    // allocate player
    Mob *player;
    player = malloc(sizeof(Mob));
    dungeon->player = player;

    // handle out of memory case
    if (player == NULL)
    {
        free(dungeon);

        return NULL;
    }

    // initialize player
    player->type = MOB_PLAYER;
    player->symbol = '@';
    player->hp = 10;
    player->maxHP = 10;
    player->minDamage = 1;
    player->maxDamage = 3;
    player->attrs.resting = 0;
    player->attrs.inMenu = 0;
    player->equipment.weapon = NULL;
    player->equipment.armor = NULL;
    player->attrs.exp = 0;
    player->attrs.expNext = 1000;
    player->attrs.level = 1;
    player->attrs.running = direction(0, 0);

    // give player some simple equipment
    Item *armor = leather();
    Item *weapon = quarterstaff();
    player->equipment.armor = move_item(armor, &player->items);
    player->equipment.weapon = move_item(weapon, &player->items);

    // initialize first level
    Level *level = create_level(1);
    dungeon->level = level;

    // handle out of memory case
    if (level == NULL)
    {
        free(player);
        free(dungeon);

        return NULL;
    }

    return dungeon;
}

int max_depth(Dungeon *dungeon)
{
    Level *cur = dungeon->level;
    while (cur->next)
        ++cur;

    return cur->depth;
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

// dungeon generation stuff

typedef struct {
    int w;
    int h;
} Dimensions;

typedef struct {
    Coords coords;  // starting at top left corner of box
    Dimensions dimensions;
} Box;


// FIXME these two functions are a quick fix
Coords empty_coords()
{
    Coords coords;
    coords.x = MAX_WIDTH;
    coords.y = MAX_HEIGHT;

    return coords;
}
int is_empty(Coords coords)
{
    return coords.x == MAX_WIDTH && coords.y == MAX_HEIGHT;
}

Box *random_cell(Level *level);
void fill_cell(Box *cell, Level *level, int fill);
void randomly_fill_corridors(Level *level, const Box **cells, int startIndex, int cellCount);
int branches(const Box *cell, const Level *level);
void randomly_fill_tiles(Level *level)
{
    int maxCells = generate(MIN_CELLS + 1, MAX_CELLS - 1);
    int cellCount = 0;
    int i = 0;

    // randomly fill cells
    Box **cells;
    cells = malloc(sizeof(Box*) * maxCells);
    memset(cells, 0, sizeof(Box*) * maxCells);
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

    // randomly place upstairs
    i = generate(0, cellCount - 1);
    while (branches(cells[i], level) == 0) // ensure we pick valid cell for upstairs
        i = generate(0, cellCount - 1);
    Coords up;
    up.x = cells[i]->coords.x + cells[i]->dimensions.w/2;
    up.y = cells[i]->coords.y + cells[i]->dimensions.h/2;
    level->tiles[up.y][up.x].type = TILE_STAIR_UP;

    // randomly place downstairs
    // TODO place downstairs at greater distance from upstairs
    // TODO once win condition is defined, don't place downstairs on last level
    int j = i;
    Coords down;
    while (i == j || branches(cells[j], level) == 0) // ensure we pick valid cell for downstairs
    {
        j = generate(0, cellCount - 1);
        down.x = cells[j]->coords.x + cells[j]->dimensions.w/2;
        down.y = cells[j]->coords.y + cells[j]->dimensions.h/2;

        // try again if up & down stairs don't connect, TODO endless loop?
        const Coords **path = find_path(up, down, (const Tile**) level->tiles);
        if (path == NULL)
            j = i;
        else
            free_path(path);
    }
    level->tiles[down.y][down.x].type = TILE_STAIR_DOWN;

    // free cells
    for (i = 0; i < cellCount; ++i)
        free(cells[i]);
    free(cells);
}

void randomly_fill_mobs(Level *level, int max)
{
    Mob **mobs = level->mobs;

    int mobIndex = 0;
    int amount = generate(0, max);
    for (int i = 0; i < amount; ++i)
    {
        Coords coords = random_passable_coords(level);
        Mob *mob = create_mob(level->depth, coords);

        if (mob == NULL)
            return;

        // don't spawn mobs on stairs
        while (get_tile(level, coords)->type == TILE_STAIR_UP ||
                get_tile(level, coords)->type == TILE_STAIR_DOWN)
        {
            coords = random_passable_coords(level);
        }

        insert_mob(mob, mobs);
    }
}

int is_path_clear(const Coords start, const Coords target, const Box **cells, int cellCount);
void draw_line(const Coords a, const Coords b, Level *level);
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

    // check for possible neighbor, and draw line to its closest wall
    for (int i = 0; i < cellCount; ++i) // check cells other than start cell
    {
        // neighbor can have at most 1 branch
        if (branches(cells[i], level) > 1)
            continue;

        // check we're not comparing same cell
        if (i == startIndex)
            continue;

        // try *all* walls to target
        Coords start;
        for (int j = 1; j <= 4; ++j)
        {
            switch (j)
            {
                case 1:
                    // left wall
                    start.x = cells[startIndex]->coords.x;
                    start.y = cells[startIndex]->coords.y + (cells[startIndex]->dimensions.h / 2);
                    break;
                case 2:
                    // top wall
                    start.x = cells[startIndex]->coords.x + (cells[startIndex]->dimensions.w / 2);
                    start.y = cells[startIndex]->coords.y;
                    break;
                case 3:
                    // right wall
                    start.x = cells[startIndex]->coords.x + cells[startIndex]->dimensions.w - 1;
                    start.y = cells[startIndex]->coords.y + (cells[startIndex]->dimensions.h / 2);
                    break;
                case 4:
                    // bottom wall
                    start.x = cells[startIndex]->coords.x + (cells[startIndex]->dimensions.w / 2);
                    start.y = cells[startIndex]->coords.y + cells[startIndex]->dimensions.h - 1;
                    break;
            }

            Coords target;
            target.x = cells[i]->coords.x + (cells[i]->dimensions.w / 2);
            target.y = cells[i]->coords.y + (cells[i]->dimensions.h / 2);

            // pick neighbor's closest wall
            if (j == 1) // left
                target.x = cells[i]->coords.x + cells[i]->dimensions.w - 1; // right
            else if (j == 2) // top
                target.y = cells[i]->coords.y + cells[i]->dimensions.h - 1; // bottom
            else if (j == 3) // right
                target.x = cells[i]->coords.x; // left
            else if (j == 4) // bottom
                target.y = cells[i]->coords.y; // top

            // check for clear line from start to target
            if (is_path_clear(start, target, cells, cellCount))
            {
                draw_line(start, target, level);
                break;
            }
        }
    }

    return randomly_fill_corridors(level, cells, startIndex + 1, cellCount);
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

// draws a line using straight lines (just a simple right angle for now)
// TODO should probably avoid carving into walls
void draw_line(const Coords a, const Coords b, Level *level)
{
    // x & y direction
    int dy = 0, dx = 0;

    // pick direction based on greatest difference between coords
    if (abs(a.y - b.y) > abs(a.x - b.x))
    {
        if (a.y > b.y)
            dy = -1; // go up
        else
            dy = 1; // go down
    }
    else
    {
        if (a.x > b.x)
            dx = -1; // go left
        else
            dx = 1; // go right
    }

    // draw caverns in direction of b
    Coords current;
    current.x = a.x;
    current.y = a.y;
    do
    {
        if (get_tile(level, current) == NULL)
            return;

        if (level->tiles[current.y][current.x].type != TILE_FLOOR)
            level->tiles[current.y][current.x].type = TILE_CAVERN;

        /* if (current.y == b.y && dy != 0) */
        /* { */
        /*     dy = 0; */
        /*     if (current.x > b.x) */
        /*         dx = -1; // go left */
        /*     else if (current.x < b.x) */
        /*         dx = 1; // go right */
        /* } */
        /* else if (current.x == b.x && dx != 0) */
        /* { */
        /*     dx = 0; */
        /*     if (current.y > b.y) */
        /*         dy = -1; // go up */
        /*     else if (current.y < b.y) */
        /*         dy = 1; // go down */
        /* } */

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

        current.y += dy;
        current.x += dx;
    }
    while (current.x != b.x || current.y != b.y);
}

Coords random_coords(Level *level)
{
    Coords coords;
    coords.x = generate(0, MAX_WIDTH - 1);
    coords.y = generate(0, MAX_HEIGHT - 1);

    return coords;
}

Coords random_open_coords(Level *level)
{
    // do simple brute force attempt to get an open coord
    int i = 0;
    while (i < MAX_RANDOM_RECURSION)
    {
        Coords coords = random_coords(level);
        const Tile *t = get_tile(level, coords);
        if (t != NULL && t->type == TILE_NONE)
            return coords;
        ++i;
    }

    return empty_coords();
}

Coords random_passable_coords(Level *level)
{
    // do simple brute force attempt to get a passable coord
    int i = 0;
    while (i < MAX_RANDOM_RECURSION)
    {
        Coords coords = random_coords(level);
        const Tile *t = get_tile(level, coords);
        Mob *m = get_mob(level, coords);
        if (t != NULL && is_passable(*t) && m == NULL)
            return coords;
        ++i;
    }

    return empty_coords();
}

Dimensions random_dimensions();
Box *random_cell(Level *level)
{
    Coords coords;
    int impassable, i;
    Dimensions dimensions;

    do
    {
        coords = random_open_coords(level);

        if (is_empty(coords))
            return NULL;

        dimensions = random_dimensions();

        // check all tiles if passable from coords -> dimension
        impassable = 0;
        for (int y = coords.y; y < coords.y + dimensions.h; ++y)
        {
            for (int x = coords.x; x < coords.x + dimensions.w; ++x)
            {
                const Tile *t = get_tile(level, xy(x, y));
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

    int i = generate(0, 14);

    Dimensions dimensions;
    dimensions.w = possibleCellDimensions[i][0];
    dimensions.h = possibleCellDimensions[i][1];

    return dimensions;
}

// checks for a clear straight line from center of start to center of target
// return 1 if possible neighbor
int within_cell(const Coords c, const Box cell);
const Coords **get_line(const Coords a, const Coords b);
void free_path(const Coords **line);
int is_path_clear(const Coords start, const Coords target, const Box **cells, int cellCount)
{
    // draw straight line from start -> target
    const Coords **line = get_line(start, target);

    // check for any coord that passes through another cell
    int i = 0;
    while (line[i] != NULL)
    {
        // ensure we're not comparing start or target
        if (!(line[i]->x == start.x && line[i]->y == start.y) && !(line[i]->x == target.x && line[i]->y == target.y))
        {
            for (int j = 0; j < cellCount; ++j)
            {
                const Box *cell = cells[j];

                // if we're within *another* cell, this is no neighbor
                if (within_cell(*line[i], *cell))
                {
                    free_path(line);

                    return 0;
                }
            }
        }
        ++i;
    }

    free_path(line);

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
                const Tile *t = get_tile(level, xy(x, y));
                if (t != NULL && t->type != TILE_NONE && t->type != TILE_WALL && t->type != TILE_WALL_SIDE)
                    ++count;
            }
        }

    return count;
}
