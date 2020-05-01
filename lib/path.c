#include "path.h"

#include <stdlib.h>
#include <memory.h>

Coords xy(int x, int y)
{
    Coords c;
    c.x = x;
    c.y = y;

    return c;
}

// get straight line from a to b
const Coords **get_line(const Coords a, const Coords b, int maxLength)
{
    Coords **line;
    line = malloc(sizeof(Coords*)*maxLength);
    memset(line, 0, sizeof(Coords*)*maxLength);

    Coords currentCoords = a;
    for (int i = 0; i < maxLength; ++i)
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

void free_path(const Coords **line, int maxLength)
{
    int i = 0;
    for (int i = 0; i < maxLength; ++i)
    {
        if (line[i] != NULL)
            free((Coords*) line[i]);
    }
    free((Coords**) line);
}

/**
 * Legacy code below, need to convert it into platform-agnostic code.

int pathfind(const Coords start, const Coords end, const Tile **tiles, int **walked, int **path);
int **create_walked(int **previous);
void free_walked(int **walked);
const Coords **find_path(const Coords start, const Coords end, const Tile **tiles)
{
    int **walked = create_walked(NULL);
    int **path = create_walked(NULL);

    if (!pathfind(xy(start.x, start.y), end, tiles, walked, path))
    {
        free_walked(walked);
        free_walked(path);

        return NULL;
    }

    free_walked(walked);

    // allocate memory for result path
    Coords **result;
    result = malloc(sizeof(Coords*)*MAX_WIDTH*MAX_HEIGHT);
    memset(result, 0, sizeof(Coords*)*MAX_WIDTH*MAX_HEIGHT);

    // create Coords from path
    Coords **cur = result;
    for (int y = 0; y < MAX_HEIGHT; ++y)
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            if (path[y][x])
            {
                *cur = malloc(sizeof(Coords));
                (*cur)->x = x;
                (*cur)->y = y;
                ++cur;
            }
        }

    free_walked(path);

    return (const Coords **) result;
}

// core pathfinding function, updates walked with newly walked coords
// returns 0 if no path, or 1 if path found
int pathfind(const Coords start, const Coords end, const Tile **tiles, int **walked, int **path)
{
    // out of bounds check
    if (start.y >= MAX_HEIGHT || start.x >= MAX_WIDTH || start.y < 0 || start.x < 0)
        return 0;

    // return 0 if we already checked this tile or if it is impassable
    if (walked[start.y][start.x] == 1 || !is_passable((Tile) tiles[start.y][start.x]))
        return 0;

    // needs paths to be initialized
    if (walked == NULL || path == NULL)
        return 0;

    walked[start.y][start.x] = 1;
    path[start.y][start.x] = 1;

    // check for success
    if (start.x == end.x && start.y == end.y)
        return 1;

    // check path in 4 directions to end
    Coords cur;
    for (int i = 0; i < 4; ++i)
    {
        // set current coordinate
        if (i == 0)
            cur = xy(start.x + 1, start.y);
        else if (i == 1)
            cur = xy(start.x - 1, start.y);
        else if (i == 2)
            cur = xy(start.x, start.y + 1);
        else
            cur = xy(start.x, start.y - 1);

        int **nextPath = create_walked(path);

        if (pathfind(cur, end, tiles, walked, nextPath))
        {
            // assign nextPath values to path
            for (int y = 0; y < MAX_HEIGHT; ++y)
                for (int x = 0; x < MAX_WIDTH; ++x)
                    path[y][x] = nextPath[y][x];

            free_walked(nextPath);

            return 1;
        }

        free_walked(nextPath);
    }

    return 0;
}

int **create_walked(int **previous)
{
    int **walked = malloc(sizeof(int*) * MAX_HEIGHT);
    memset(walked, 0, sizeof(int*) * MAX_HEIGHT);
    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        walked[y] = malloc(sizeof(int) * MAX_WIDTH);
        memset(walked[y], 0, sizeof(int) * MAX_WIDTH);
        if (previous != NULL && previous[y] != NULL)
            for (int x = 0; x < MAX_WIDTH; ++x)
                walked[y][x] = previous[y][x];
    }

    return walked;
}

void free_walked(int **walked)
{
    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        free(walked[y]);
    }
    free(walked);
}

*/
