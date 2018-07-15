#include "random.h"
#include <memory.h>
#include <stdlib.h>

Coords xy(int x, int y)
{
    Coords c;
    c.x = x;
    c.y = y;

    return c;
}

int pathfind(int x, int y, const Coords end, const Level *level, int **walked);
int find_path(const Coords start, const Coords end, const Level *level)
{
    // allocate array of walked tiles
    int **walked;
    walked = malloc(sizeof(int*) * MAX_HEIGHT);
    memset(walked, 0, sizeof(int*) * MAX_HEIGHT);
    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        walked[y] = malloc(sizeof(int) * MAX_WIDTH);
        memset(walked[y], 0, sizeof(int) * MAX_WIDTH);
    }

    int result = pathfind(start.x, start.y, end, level, walked);

    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        free(walked[y]);
    }
    free(walked);

    return result;
}

// core pathfinding function, updates walked with newly walked coords
// returns 0 if no path, or 1 if path found
int pathfind(int x, int y, const Coords end, const Level *level, int **walked)
{
    // out of bounds check
    if (y >= MAX_HEIGHT || x >= MAX_WIDTH || y < 0 || x < 0)
        return 0;

    // return 0 if we already checked this tile or if it is impassable
    if (walked[y][x] == 1 || !is_passable((Tile) level->tiles[y][x]))
        return 0;

    walked[y][x] = 1;

    // check for success
    if (x == end.x && y == end.y)
        return 1;

    if (pathfind(x + 1, y, end, level, walked) > 0 ||
            pathfind(x - 1, y, end, level, walked) > 0 ||
            pathfind(x, y + 1, end, level, walked) > 0 ||
            pathfind(x, y - 1, end, level, walked) > 0)
        return 1;
    else
        return 0;
}

// get straight line from a to b
const Coords **get_line(const Coords a, const Coords b)
{
    Coords **line;
    line = malloc(sizeof(Coords*)*MAX_WIDTH+MAX_HEIGHT);
    memset(line, 0, sizeof(Coords*)*MAX_WIDTH+MAX_HEIGHT);

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
    for (int i = 0; i < MAX_WIDTH + MAX_HEIGHT; ++i)
    {
        if (line[i] == NULL)
            break;
        free((Coords*) line[i]);
        ++i;
    }
    free((Coords**) line);
}
