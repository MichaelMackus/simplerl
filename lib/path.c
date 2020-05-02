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
    if (path == NULL)
        return;
    RL_Path *current = path->head;
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

// TODO add getters for height/width
struct _RL_Map {
    int width;
    int height;
    char *tiles; // 2d array of tiles, set to 1 if tile is passable
};

int pathfind(const RL_Coords start,
             const RL_Coords end,
             const RL_Map *map,
             double diagonal_distance,
             double (*heuristic_func)(RL_Coords node),
             double **walked, RL_Path *path);
double **create_walked(double **previous, int width, int height);
void free_walked(double **walked, int height);
RL_Path *find_path(const RL_Coords start,
                   const RL_Coords end,
                   const RL_Map *map,
                   double diagonal_distance,
                   double (*heuristic_func)(RL_Coords node))
{
    double **walked = create_walked(NULL, map->width, map->height);
    RL_Path *path = malloc(sizeof(struct _RL_Path));
    path->next = NULL;
    path->current = path;
    path->head = path;

    if (!pathfind(start, end, map, diagonal_distance, heuristic_func, walked, path))
    {
        free_walked(walked, map->height);
        rl_clear_path(path);

        return NULL;
    }

    free_walked(walked, map->height);

    return path;
}

// core pathfinding function, updates walked with newly walked coords
// returns 0 if no path, or 1 if path found
int pathfind(const RL_Coords start,
             const RL_Coords end,
             const RL_Map *map,
             double diagonal_distance,
             double (*heuristic_func)(RL_Coords node),
             double **walked, RL_Path *path)
{
    // out of bounds check
    if (start.y >= map->height || start.x >= map->width || start.y < 0 || start.x < 0)
        return 0;

    // return 0 if we already checked this tile or if it is impassable
    if (walked[start.y][start.x] == 1 || !rl_is_passable(map, start.x, start.y))
        return 0;

    // needs paths to be initialized
    if (walked == NULL || path == NULL)
        return 0;

    walked[start.y][start.x] = 1.0;
    path->loc = start;

    // check for success
    if (start.x == end.x && start.y == end.y)
        return 1;

    // check path in 4 directions to end
    RL_Coords nextLoc;
    for (int i = 0; i < 4; ++i)
    {
        // set current coordinate
        if (i == 0)
            nextLoc = rl_coords(start.x + 1, start.y);
        else if (i == 1)
            nextLoc = rl_coords(start.x - 1, start.y);
        else if (i == 2)
            nextLoc = rl_coords(start.x, start.y + 1);
        else
            nextLoc = rl_coords(start.x, start.y - 1);

        RL_Path *nextPath = malloc(sizeof(struct _RL_Path));
        if (nextPath == NULL)
            return 0;

        nextPath->next = NULL;
        nextPath->head = NULL;
        nextPath->current = NULL;

        if (pathfind(nextLoc, end, map, diagonal_distance, heuristic_func, walked, nextPath))
        {
            // assign nextPath values to path
            path->next = nextPath;

            return 1;
        }

        free(nextPath);
    }

    return 0;
}

double **create_walked(double **previous, int width, int height)
{
    double **walked = malloc(sizeof(double*) * height);
    memset(walked, 0, sizeof(double*) * height);
    for (int y = 0; y < height; ++y)
    {
        walked[y] = malloc(sizeof(double) * width);
        memset(walked[y], 0, sizeof(double) * width);
        if (previous != NULL && previous[y] != NULL)
            for (int x = 0; x < width; ++x)
                walked[y][x] = previous[y][x];
    }

    return walked;
}

void free_walked(double **walked, int height)
{
    if (walked == NULL) return;
    for (int y = 0; y < height; ++y)
    {
        free(walked[y]);
    }
    free(walked);
}
