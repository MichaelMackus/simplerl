#include "dungeon.h"
#include <stdlib.h>
#include <memory.h>

typedef struct {
    int xdir;
    int ydir;
} Direction;

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
    player->equipment.weapon = NULL;
    player->equipment.armor = NULL;
    player->attrs.exp = 0;
    player->attrs.expNext = 1000;
    player->attrs.level = 1;

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
    rl_coords coords;  // starting at top left corner of box
    Dimensions dimensions;
} Box;


// FIXME these two functions are a quick fix
rl_coords empty_coords()
{
    rl_coords coords;
    coords.x = MAX_WIDTH;
    coords.y = MAX_HEIGHT;

    return coords;
}
int is_empty(rl_coords coords)
{
    return coords.x == MAX_WIDTH && coords.y == MAX_HEIGHT;
}

Box *random_cell(Level *level);
void fill_cell(Box *cell, Level *level, int fill);
void randomly_fill_corridors(Level *level, const Box **cells, int cellCount);
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
    randomly_fill_corridors(level, (const Box**) cells, cellCount);

    // randomly place upstairs
    i = generate(0, cellCount - 1);
    rl_coords up;
    up.x = cells[i]->coords.x + cells[i]->dimensions.w/2;
    up.y = cells[i]->coords.y + cells[i]->dimensions.h/2;
    level->tiles[up.y][up.x].type = TILE_STAIR_UP;

    // randomly place downstairs
    // TODO place downstairs at greater distance from upstairs
    // TODO once win condition is defined, don't place downstairs on last level
    int j = i;
    while (i == j)
        i = generate(0, cellCount - 1);
    rl_coords down;
    down.x = cells[i]->coords.x + cells[i]->dimensions.w/2;
    down.y = cells[i]->coords.y + cells[i]->dimensions.h/2;
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
        rl_coords coords = random_passable_coords(level);
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

int is_path_clear(const rl_coords start, const rl_coords target, const Box **cells, int cellCount);
void draw_line(const rl_coords a, const rl_coords b, Level *level);
int branches(const Box *cell, const Level *level);
rl_coords find_random_section(const Box *cell);
rl_coords get_section(const Box *cell, Direction dir);
void randomly_fill_corridors(Level *level, const Box **cells, int cellCount)
{
    // ensure at least 1 cell has no branches
    int unreachableCell = 0;
    for (int i = 0; i < cellCount; ++i) {
        if (branches(cells[i], level) == 0) {
            unreachableCell = 1;
            break;
        }
    }

    if (unreachableCell == 0) {
        // done
        return;
    }

    int a = generate(0, cellCount - 1);
    int b = generate(0, cellCount - 1);

    if (a != b) {
        Direction dir = { 0, 0 };
        if (abs(cells[a]->coords.x - cells[b]->coords.x) > abs(cells[a]->coords.y - cells[b]->coords.y))
            if (cells[b]->coords.x > cells[a]->coords.x)
                dir.xdir = 1;
            else
                dir.xdir = -1;
        else
            if (cells[b]->coords.y > cells[a]->coords.y)
                dir.ydir = 1;
            else
                dir.ydir = -1;

        rl_coords start = get_section(cells[a], dir);
        dir.xdir *= -1;
        dir.ydir *= -1;
        rl_coords target = get_section(cells[b], dir);

        draw_line(start, target, level);
    }

    randomly_fill_corridors(level, cells, cellCount);
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

static inline int is_wall(int type)
{
    return type == TILE_WALL || type == TILE_WALL_SIDE;
}

// draws a line using straight lines (just a simple right angle for now)
void draw_line(const rl_coords a, const rl_coords b, Level *level)
{
    // x & y direction
    int dy = 0, dx = 0;

    // pick direction based on greatest difference between coords
    int diffy = abs(a.y - b.y), diffx = abs(a.x - b.x);
    if (diffy > diffx)
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
    rl_coords current;
    current.x = a.x;
    current.y = a.y;

    while (1 == 1)
    {
        if (get_tile(level, current) == NULL)
            return;

        // go around the corner or other impassable feature
        if (level->tiles[current.y][current.x].generatorFlags & GENERATOR_IMPASSABLE)
        {
            if (current.x == b.x && current.y == b.y)
                return;

            // TODO what about when there's a turn?
            // TODO perhaps switch directions here & let code in next
            // TODO block properly handle all direction corrections
            int newdy = 0, newdx = 0;
            if (dy == 0)
            {
                if (get_tile(level, (rl_coords) { current.x, current.y + 1 }) && 
                    (level->tiles[current.y + 1][current.x].type == TILE_NONE || level->tiles[current.y + 1][current.x].type == TILE_CAVERN))
                    newdy = 1;
                else if (get_tile(level, (rl_coords) { current.x, current.y - 1 }) && 
                    (level->tiles[current.y - 1][current.x].type == TILE_NONE || level->tiles[current.y - 1][current.x].type == TILE_CAVERN))
                    newdy = -1;
                else return;
            }
            else if (dx == 0)
            {
                if (get_tile(level, (rl_coords) { current.x + 1, current.y }) && 
                    (level->tiles[current.y][current.x + 1].type == TILE_NONE || level->tiles[current.y][current.x + 1].type == TILE_CAVERN))
                    newdx = 1;
                else if (get_tile(level, (rl_coords) { current.x - 1, current.y }) && 
                    (level->tiles[current.y][current.x - 1].type == TILE_NONE || level->tiles[current.y][current.x - 1].type == TILE_CAVERN))
                    newdx = -1;
                else
                    return; // error, abort
            }

            current.x -= dx;
            current.y -= dy;
            current.x += newdx;
            current.y += newdy;
        }
        else
        {
            // if this is a wall, mark the walls *next* to it impassable
            // (so we don't carve double wide doorways)
            if (is_wall(level->tiles[current.y][current.x].type))
            {
                if (get_tile(level, (rl_coords) { current.x + 1, current.y }) && is_wall(level->tiles[current.y][current.x + 1].type))
                    level->tiles[current.y][current.x + 1].generatorFlags = GENERATOR_IMPASSABLE;
                if (get_tile(level, (rl_coords) { current.x - 1, current.y }) && is_wall(level->tiles[current.y][current.x - 1].type))
                    level->tiles[current.y][current.x - 1].generatorFlags = GENERATOR_IMPASSABLE;
                if (get_tile(level, (rl_coords) { current.x, current.y + 1 }) && is_wall(level->tiles[current.y + 1][current.x].type))
                    level->tiles[current.y + 1][current.x].generatorFlags = GENERATOR_IMPASSABLE;
                if (get_tile(level, (rl_coords) { current.x, current.y - 1 }) && is_wall(level->tiles[current.y - 1][current.x].type))
                    level->tiles[current.y - 1][current.x].generatorFlags = GENERATOR_IMPASSABLE;
            }

            if (level->tiles[current.y][current.x].type == TILE_NONE)
                level->tiles[current.y][current.x].type = TILE_CAVERN;
            else if (is_wall(level->tiles[current.y][current.x].type))
                level->tiles[current.y][current.x].type = TILE_FLOOR;

            if (current.x == b.x && current.y == b.y)
                return;

            if (current.y == b.y && dy != 0)
            {
                dy = 0;
                if (current.x > b.x)
                    dx = -1; // go left
                else if (current.x < b.x)
                    dx = 1; // go right
            }
            else if (current.x == b.x && dx != 0)
            {
                dx = 0;
                if (current.y > b.y)
                    dy = -1; // go up
                else if (current.y < b.y)
                    dy = 1; // go down
            }

            current.y += dy;
            current.x += dx;
        }
    }
}

rl_coords find_random_section(const Box *cell)
{
    rl_coords result;

    // pick random wall
    int j = generate(1, 4);

    switch (j)
    {
        case 1:
            // left wall
            result.x = cell->coords.x;
            result.y = cell->coords.y + (cell->dimensions.h / 2);
            break;
        case 2:
            // top wall
            result.x = cell->coords.x + (cell->dimensions.w / 2);
            result.y = cell->coords.y;
            break;
        case 3:
            // right wall
            result.x = cell->coords.x + cell->dimensions.w - 1;
            result.y = cell->coords.y + (cell->dimensions.h / 2);
            break;
        case 4:
            // bottom wall
            result.x = cell->coords.x + (cell->dimensions.w / 2);
            result.y = cell->coords.y + cell->dimensions.h - 1;
            break;
    }

    return result;
}

rl_coords get_section(const Box *cell, Direction dir)
{
    rl_coords result;

    if (dir.xdir < 0)
    {
        // left wall
        result.x = cell->coords.x;
        result.y = cell->coords.y + (cell->dimensions.h / 2);
    }

    if (dir.xdir > 0)
    {
        // right wall
        result.x = cell->coords.x + cell->dimensions.w - 1;
        result.y = cell->coords.y + (cell->dimensions.h / 2);
    }

    if (dir.ydir < 0)
    {
        // top wall
        result.x = cell->coords.x + (cell->dimensions.w / 2);
        result.y = cell->coords.y;
    }

    if (dir.ydir > 0)
    {
        // bottom wall
        result.x = cell->coords.x + (cell->dimensions.w / 2);
        result.y = cell->coords.y + cell->dimensions.h - 1;
    }

    return result;
}

rl_coords random_coords(Level *level)
{
    rl_coords coords;
    coords.x = generate(0, MAX_WIDTH - 1);
    coords.y = generate(0, MAX_HEIGHT - 1);

    return coords;
}

rl_coords random_open_coords(Level *level)
{
    // do simple brute force attempt to get an open coord
    int i = 0;
    while (i < MAX_RANDOM_RECURSION)
    {
        rl_coords coords = random_coords(level);
        const Tile *t = get_tile(level, coords);
        if (t != NULL && t->type == TILE_NONE)
            return coords;
        ++i;
    }

    return empty_coords();
}

rl_coords random_passable_coords(Level *level)
{
    // do simple brute force attempt to get a passable coord
    int i = 0;
    while (i < MAX_RANDOM_RECURSION)
    {
        rl_coords coords = random_coords(level);
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
    rl_coords coords;
    int impassable, i;
    Dimensions dimensions;

    do
    {
        coords = random_open_coords(level);

        if (is_empty(coords))
            return NULL;

        dimensions = random_dimensions();

        // check all tiles if passable from coords -> dimension, with space of 1 on each side
        impassable = 0;
        for (int y = coords.y - 1; y < coords.y + dimensions.h + 1; ++y)
        {
            for (int x = coords.x - 1; x < coords.x + dimensions.w + 1; ++x)
            {
                if (y < 0 || x < 0)
                    continue;
                const Tile *t = get_tile(level, XY(x, y));
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
            {
                if (y == cell->coords.y || y == cell->coords.y + cell->dimensions.h - 1)
                    t = create_tile(TILE_WALL);
                else if (x == cell->coords.x || x == cell->coords.x + cell->dimensions.w - 1)
                    t = create_tile(TILE_WALL_SIDE);
                else
                    t = create_tile(TILE_FLOOR);

                // if this is a corner, mark impassable for generation
                if ((y == cell->coords.y || y == cell->coords.y + cell->dimensions.h - 1) &&
                    (x == cell->coords.x || x == cell->coords.x + cell->dimensions.w - 1))
                    t.generatorFlags = GENERATOR_IMPASSABLE;
            }
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
                const Tile *t = get_tile(level, XY(x, y));
                if (t != NULL && t->type != TILE_NONE && t->type != TILE_WALL && t->type != TILE_WALL_SIDE)
                    ++count;
            }
        }

    return count;
}
