#include "path.h"

#include <stdlib.h>
#include <memory.h>

struct _RL_Path {
    RL_Coords loc; // location in the path
    struct _RL_Path *next; // next part of the linked list
    struct _RL_Path *current; // current part of path (used during iteration after the path is generated)
    struct _RL_Path *head; // head of list (used for reset & reversal)
};

RL_Path *rl_get_line(const RL_Coords a, const RL_Coords b)
{
    RL_Path *head = malloc(sizeof(struct _RL_Path));

    if (head == NULL)
        return NULL;

    head->next = NULL;
    head->current = head;
    head->head = head;

    int deltaX = abs(a.x - b.x);
    int xIncrement = b.x > a.x ? 1 : -1;
    int deltaY = abs(a.y - b.y);
    int yIncrement = b.y > a.y ? 1 : -1;
    double error = 0.0;
    double slope = deltaX ? (double)deltaY / (double)deltaX : 0.0;

    RL_Path *path = head;
    path->loc = a;
    while (path->loc.x != b.x || path->loc.y != b.y)
    {
        RL_Coords currentCoords = path->loc;

        if (deltaX > deltaY)
        {
            error += slope;
            if (error > 0.5 && currentCoords.y != b.y)
            {
                error -= 1.0;
                currentCoords.y += yIncrement;
            }

            currentCoords.x += xIncrement;
        }
        else
        {
            error += 1/slope;
            if (error > 0.5 && currentCoords.x != b.x)
            {
                error -= 1.0;
                currentCoords.x += xIncrement;
            }

            currentCoords.y += yIncrement;
        }

        // add new member to linked list & advance
        path->next = malloc(sizeof(struct _RL_Path));

        if (path->next == NULL)
            return head;

        path->next->loc = currentCoords;
        path->next->next = NULL;
        path = path->next;
    }

    return head;
}

void rl_clear_path(RL_Path *path)
{
    RL_Path *current = path;
    RL_Path *next;
    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }
}

const RL_Coords *rl_walk_path(RL_Path *path)
{
    RL_Path *current = path->current;

    if (current == NULL)
        return NULL;

    path->current = current->next;

    return &(current->loc);
}

void rl_reset_path(RL_Path *path)
{
    path->current = path->head;
}

void rl_reverse_path(RL_Path *path)
{
    RL_Path *prev = NULL; 
    RL_Path *current = path->head;
    RL_Path *next = NULL;
    while (current != NULL) { 
        // Store next 
        next = current->next; 

        // Reverse current node's pointer 
        current->next = prev; 

        // Move pointers one position ahead. 
        prev = current; 
        current = next; 
    }

    path->head = prev;
    path->current = prev;
}

/**
 * Legacy code below, need to convert it into platform-agnostic code.

int pathfind(const RL_Coords start, const RL_Coords end, const Tile **tiles, int **walked, int **path);
int **create_walked(int **previous);
void free_walked(int **walked);
const RL_Coords **find_path(const RL_Coords start, const RL_Coords end, const Tile **tiles)
{
    int **walked = create_walked(NULL);
    int **path = create_walked(NULL);

    if (!pathfind(rl_coords(start.x, start.y), end, tiles, walked, path))
    {
        free_walked(walked);
        free_walked(path);

        return NULL;
    }

    free_walked(walked);

    // allocate memory for result path
    RL_Coords **result;
    result = malloc(sizeof(RL_Coords*)*MAX_WIDTH*MAX_HEIGHT);
    memset(result, 0, sizeof(RL_Coords*)*MAX_WIDTH*MAX_HEIGHT);

    // create RL_Coords from path
    RL_Coords **cur = result;
    for (int y = 0; y < MAX_HEIGHT; ++y)
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            if (path[y][x])
            {
                *cur = malloc(sizeof(RL_Coords));
                (*cur)->x = x;
                (*cur)->y = y;
                ++cur;
            }
        }

    free_walked(path);

    return (const RL_Coords **) result;
}

// core pathfinding function, updates walked with newly walked coords
// returns 0 if no path, or 1 if path found
int pathfind(const RL_Coords start, const RL_Coords end, const Tile **tiles, int **walked, int **path)
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
    RL_Coords cur;
    for (int i = 0; i < 4; ++i)
    {
        // set current coordinate
        if (i == 0)
            cur = rl_coords(start.x + 1, start.y);
        else if (i == 1)
            cur = rl_coords(start.x - 1, start.y);
        else if (i == 2)
            cur = rl_coords(start.x, start.y + 1);
        else
            cur = rl_coords(start.x, start.y - 1);

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
