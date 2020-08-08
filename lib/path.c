#include "path.h"
#include "queue.h"
#include "util.h"

#include <assert.h>
#include <stdlib.h>
#include <memory.h>

struct rl_path {
    rl_coords loc; // location in the path
    struct rl_path *next; // next part of the linked list
    struct rl_path *current; // current part of path (used during iteration after the path is generated)
    struct rl_path *head; // head of list (used for reset & reversal)
};

rl_path *rl_get_line(const rl_coords a, const rl_coords b)
{
    rl_path *head = malloc(sizeof(struct rl_path));

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

    rl_path *path = head;
    path->loc = a;
    while (path->loc.x != b.x || path->loc.y != b.y) {
        rl_coords currentCoords = path->loc;

        if (deltaX > deltaY) {
            error += slope;
            if (error > 0.5 && currentCoords.y != b.y) {
                error -= 1.0;
                currentCoords.y += yIncrement;
            }

            currentCoords.x += xIncrement;
        } else {
            error += 1/slope;
            if (error > 0.5 && currentCoords.x != b.x) {
                error -= 1.0;
                currentCoords.x += xIncrement;
            }

            currentCoords.y += yIncrement;
        }

        // add new member to linked list & advance
        path->next = malloc(sizeof(struct rl_path));

        // TODO free
        if (path->next == NULL)
            return NULL;

        path->next->loc = currentCoords;
        path->next->next = NULL;
        path = path->next;
    }

    return head;
}

int all_passable(rl_coords node, void *user_data)
{
    return 1;
}
rl_path *rl_get_line_manhattan(const rl_coords a, const rl_coords b)
{

    return rl_find_path_cb(a, b, 0, &manhattan_distance, &all_passable, NULL);
}

void rl_clear_path(rl_path *path)
{
    if (path == NULL)
        return;
    rl_path *current = path->head;
    rl_path *next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
}

const rl_coords *rl_walk_path(rl_path *path)
{
    if (path == NULL)
        return NULL;

    rl_path *current = path->current;

    if (current == NULL)
        return NULL;

    path->current = current->next;

    return &(current->loc);
}

void rl_reset_path(rl_path *path)
{
    path->current = path->head;
}

void rl_reverse_path(rl_path *path)
{
    rl_path *prev = NULL;
    rl_path *current = path->head;
    rl_path *next = NULL;
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

typedef struct rl_node {
    rl_coords loc;
    double f, g;
    struct rl_node *parent;
} rl_node;

// Global variables used for pathfinding. These are here since we don't want to
// malloc for *each* new path, and only need to realloc if the size is too big
static rl_queue *openSet;
static rl_queue *closedSet;

rl_node *find_node(rl_queue *queue, rl_coords loc)
{
    rl_queue *cur = queue;
    while (cur != NULL) {
        rl_node *n = cur->data;
        if (n && n->loc.x == loc.x && n->loc.y == loc.y)
            return n;
        cur = cur->next;
    }

    return NULL;
}

// default passable func that checks if a map tile is passable
int is_map_passable(rl_coords node, void *user_data)
{
    rl_map *map = user_data;

    return rl_is_passable(map, node);
}

rl_path *rl_find_path(const rl_coords start,
                        const rl_coords end,
                        const rl_map *map,
                        double diagonal_distance,
                        rl_heuristic_f heuristic)
{
    return rl_find_path_cb(start, end, diagonal_distance, heuristic,
                           &is_map_passable, (void *) map);
}

rl_path *rl_find_path_cb(const rl_coords start,
                           const rl_coords end,
                           double diagonal_distance,
                           rl_heuristic_f heuristic,
                           rl_passable_f  is_passable,
                           void *user_data)
{
    // push start node to open set
    rl_node *curNode = calloc(1, sizeof(rl_node));
    curNode->loc = start;
    rl_push(&openSet, curNode, 0);
    while (rl_peek(openSet) != NULL) {
        curNode = rl_pop(&openSet);

        // check for end condition
        if (curNode->loc.x == end.x && curNode->loc.y == end.y) {
            rl_push(&closedSet, curNode, curNode->f);
            break;
        }

        // add neighbors to open set
        rl_coords neighbors[8] = {
            { curNode->loc.x - 1, curNode->loc.y },
            { curNode->loc.x + 1, curNode->loc.y },
            { curNode->loc.x, curNode->loc.y - 1 },
            { curNode->loc.x, curNode->loc.y + 1 },
            { curNode->loc.x - 1, curNode->loc.y - 1 },
            { curNode->loc.x - 1, curNode->loc.y + 1 },
            { curNode->loc.x + 1, curNode->loc.y - 1 },
            { curNode->loc.x + 1, curNode->loc.y + 1 },
        };
        for (int i = 0; i < 8; ++i) {
            rl_coords neighborLoc = neighbors[i];
            if (is_passable(neighborLoc, user_data)) {
                double diff = abs(curNode->loc.x - neighborLoc.x) + abs(curNode->loc.y - neighborLoc.y);
                double g = curNode->g + 1.0;

                // account for diagonal distance
                if (diff > 1.0) {
                    g = curNode->g + diagonal_distance;
                    if (diagonal_distance == 0.0)
                        continue; // don't move diagonally if no distance covered
                }

                double h = heuristic ? heuristic(neighborLoc, end) : 0.0;
                double f = g + h;

                // skip if in open set & f is larger than walked f
                rl_node *n = find_node(openSet, neighborLoc);
                if (n && f >= n->f)
                    continue;

                // check if in closed set & f is larger than walked f TODO is this necessary
                n = find_node(closedSet, neighborLoc);
                if (n && f >= n->f)
                    continue;

                // add neighbor to open set
                n = malloc(sizeof(rl_node));
                n->loc = neighborLoc;
                n->f = f;
                n->g = g;
                n->parent = curNode;
                rl_push(&openSet, n, f);
            }
        }

        // finished checking current node, add to closed set & remove from open set
        rl_push(&closedSet, curNode, curNode->f);
    }

    if (
        curNode == NULL
        || !(curNode->loc.x == end.x && curNode->loc.y == end.y)
    ) {
        // free nodes in open & closed set
        rl_node *n;
        while ((n = rl_pop(&openSet)) != NULL) free(n);
        while ((n = rl_pop(&closedSet)) != NULL) free(n);

        // we could not find a path to end
        return NULL;
    }

    // make path from start to end (this traverses up the parents of
    // curLoc, making a linked list from end -> start)
    rl_path *path = NULL;
    rl_path *prevPath = NULL;
    while (curNode) {
        path = malloc(sizeof(rl_path));
        assert(path);
        path->loc = curNode->loc;
        path->next = prevPath;
        prevPath = path;
        curNode = curNode->parent;
    }
    path->head = path;
    path->current = path;

    // free nodes in open & closed set
    rl_node *n;
    while ((n = rl_pop(&openSet)) != NULL) free(n);
    while ((n = rl_pop(&closedSet)) != NULL) free(n);

    return path;
}
