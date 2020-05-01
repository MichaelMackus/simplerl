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

