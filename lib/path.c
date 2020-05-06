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

        // TODO free
        if (path->next == NULL)
            return NULL;

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
    if (path == NULL)
        return NULL;

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
    char *tiles; // 2d array of tiles, set to 1 if tile is passable; only used if passable_func is null
    int (*passable_func)(RL_Coords coords, void *user_data);
    void *user_data; // only used if passable_func
};

typedef struct _RL_Node {
    int active;
    int has_parent; // 0 for root
    int parent_x;
    int parent_y;
    double f, g;
} RL_Node;

RL_Path *rl_find_path(const RL_Coords start,
                      const RL_Coords end,
                      const RL_Map *map,
                      double diagonal_distance,
                      double (*heuristic_func)(RL_Coords node, RL_Coords end))
{
    RL_Node open[map->width][map->height];
    RL_Node closed[map->width][map->height];

    // initialize open & closed nodes to inactive (active = 0)
    memset(open, 0, sizeof(RL_Node) * map->width * map->height);
    memset(closed, 0, sizeof(RL_Node) * map->width * map->height);

    // push start node to open set
    RL_Coords curLoc = start;
    open[curLoc.x][curLoc.y].active = 1;
    while (open[curLoc.x][curLoc.y].active)
    {
        // check for end condition
        if (curLoc.x == end.x && curLoc.y == end.y)
        {
            memcpy(&closed[curLoc.x][curLoc.y], &open[curLoc.x][curLoc.y], sizeof(RL_Node));
            break;
        }

        // add neighbors to open set
        RL_Coords neighbors[8] = {
            rl_coords(curLoc.x - 1, curLoc.y),
            rl_coords(curLoc.x + 1, curLoc.y),
            rl_coords(curLoc.x, curLoc.y - 1),
            rl_coords(curLoc.x, curLoc.y + 1),
            rl_coords(curLoc.x - 1, curLoc.y - 1),
            rl_coords(curLoc.x - 1, curLoc.y + 1),
            rl_coords(curLoc.x + 1, curLoc.y - 1),
            rl_coords(curLoc.x + 1, curLoc.y + 1),
        };
        for (int i = 0; i < 8; ++i)
        {
            RL_Coords neighborLoc = neighbors[i];
            if (rl_is_passable(map, neighborLoc.x, neighborLoc.y))
            {
                double diff = abs(curLoc.x - neighborLoc.x) + abs(curLoc.y - neighborLoc.y);
                double g = open[curLoc.x][curLoc.y].g + 1.0;

                // account for diagonal distance
                if (diff > 1.0)
                {
                    g = open[curLoc.x][curLoc.y].g + diagonal_distance;
                    if (diagonal_distance == 0.0)
                        continue; // don't move diagonally if no distance covered
                }

                double h = heuristic_func ? heuristic_func(neighborLoc, end) : 0.0;
                double f = g + h;

                
                // skip if in open set & f is larger than walked f
                if (open[neighborLoc.x][neighborLoc.y].active && f >= open[neighborLoc.x][neighborLoc.y].f)
                    continue;

                // check if in closed set & f is larger than walked f
                if (closed[neighborLoc.x][neighborLoc.y].active && f >= closed[neighborLoc.x][neighborLoc.y].f)
                    continue;

                open[neighborLoc.x][neighborLoc.y].active = 1;
                open[neighborLoc.x][neighborLoc.y].has_parent = 1;
                open[neighborLoc.x][neighborLoc.y].parent_x = curLoc.x;
                open[neighborLoc.x][neighborLoc.y].parent_y = curLoc.y;
                open[neighborLoc.x][neighborLoc.y].f = f;
                open[neighborLoc.x][neighborLoc.y].g = g;
            }
        }

        // finished checking current node, add to closed set & remove from open set
        memcpy(&closed[curLoc.x][curLoc.y], &open[curLoc.x][curLoc.y], sizeof(RL_Node));
        open[curLoc.x][curLoc.y].active = 0;

        // find active node with lowest f value in open set
        for (int x = 0; x < map->width; x++)
        {
            for (int y = 0; y < map->height; y++)
            {
                if (!open[x][y].active)
                    continue;

                if (!open[curLoc.x][curLoc.y].active)
                    curLoc = rl_coords(x, y);
                else if (open[x][y].f < open[curLoc.x][curLoc.y].f)
                    curLoc = rl_coords(x, y);
            }
        }
    }

    if (curLoc.x != end.x || curLoc.y != end.y)
    {
        // we could not find a path to end
        return NULL;
    }

    // make path from start to end (this traverses up the parents of
    // curLoc, making a linked list from end -> start)
    RL_Path *path = malloc(sizeof(struct _RL_Path));
    if (path == NULL)
        return NULL;
    RL_Path *prevPath = path;
    path->loc = curLoc;
    path->next = NULL;
    while (closed[curLoc.x][curLoc.y].has_parent)
    {
        RL_Node curNode = closed[curLoc.x][curLoc.y];
        curLoc.x = curNode.parent_x;
        curLoc.y = curNode.parent_y;
        path = malloc(sizeof(struct _RL_Path));
        if (path == NULL)
            return NULL; // TODO free prev paths
        path->loc = curLoc;
        path->next = prevPath;
        prevPath = path;
    }
    path->head = path;
    path->current = path;

    return path;
}
