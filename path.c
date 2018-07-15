#include "random.h"
#include <memory.h>
#include <stdlib.h>

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
