#include "mapgen.h"
#include "path.h"
#include "util.h"

#include <assert.h>

#define MAX_GENERATOR_RECURSION 100

void rl_recursively_split_bsp(rl_bsp *root, rl_generator_f generator,
        unsigned int min_width, unsigned int min_height, float deviation, int max_recursion)
{
    if (max_recursion <= 0)
        return;

    assert(deviation >= 0.0 && deviation <= 1.0);

    unsigned int width = rl_get_bsp_width(root);
    unsigned int height = rl_get_bsp_height(root);
    unsigned int max_width = width - min_width;
    unsigned int max_height = height - min_height;

    // determine split dir & split
    rl_split_dir dir;
    if (generator(0, 1)) {
        if (width < min_width*2)
            dir = RL_SPLIT_VERTICALLY;
        else
            dir = RL_SPLIT_HORIZONTALLY;
    } else {
        if (height < min_height*2)
            dir = RL_SPLIT_HORIZONTALLY;
        else
            dir = RL_SPLIT_VERTICALLY;
    }

    unsigned int split_position;
    if (dir == RL_SPLIT_HORIZONTALLY) {
        // cannot split if current node size is too small
        if (width < min_width*2)
            return;

        unsigned int center = width / 2;
        unsigned int from = center - (center * deviation);
        unsigned int to = center + (center * deviation);
        if (from < min_width)
            from = min_width;
        if (to > max_width)
            to = max_width;

        split_position = generator(from, to);
    } else {
        // cannot split if current node size is too small
        if (height < min_height*2)
            return;

        unsigned int center = height / 2;
        unsigned int from = center - (center * deviation);
        unsigned int to = center + (center * deviation);
        if (from < min_height)
            from = min_height;
        if (to > max_height)
            to = max_height;

        split_position = generator(from, to);
    }

    rl_split_bsp(root, split_position, dir);

    // continue recursion
    rl_bsp *left = rl_get_bsp_left(root);
    rl_bsp *right = rl_get_bsp_right(root);

    if (left == NULL || right == NULL)
        return;

    rl_recursively_split_bsp(left, generator,
            min_width, min_height, deviation, max_recursion - 1);
    rl_recursively_split_bsp(right, generator,
            min_width, min_height, deviation, max_recursion - 1);
}

int is_diggable(rl_map *map, rl_map *room_map, rl_coords start, rl_coords end);
int is_corner(rl_map *map, rl_coords loc);
rl_map *rl_create_map_from_bsp(rl_bsp *root, rl_generator_f generator,
        unsigned int room_min_width, unsigned int room_min_height,
        unsigned int room_max_width, unsigned int room_max_height,
        unsigned int room_padding, rl_bsp_map_f corridor_algorithm)
{
    unsigned int map_width = rl_get_bsp_width(root);
    unsigned int map_height = rl_get_bsp_height(root);

    rl_queue *leaves = rl_get_bsp_leaves(root);
    if (leaves == NULL)
        return NULL;

    rl_queue *cur = leaves;
    size_t leafCount = 0;
    while (cur) {
        cur = cur->next;
        leafCount++;
    }

    // generate map containing original room data
    rl_map *map = rl_create_map(map_width, map_height);
    if (map == NULL)
        return NULL; // TODO free queue

    // generate rooms
    for (int i = 0; i < leafCount; ++i) {
        rl_bsp *leaf = rl_pop(&leaves);
        if (leaf == NULL) continue;

        unsigned int width = rl_get_bsp_width(leaf);
        unsigned int height = rl_get_bsp_height(leaf);
        rl_coords loc = rl_get_bsp_loc(leaf);

        unsigned int room_width = generator(room_min_width, room_max_width);
        if (room_width + room_padding*2 > width)
            room_width = width - room_padding*2;
        unsigned int room_height = generator(room_min_height, room_max_height);
        if (room_height + room_padding*2 > height)
            room_height = height - room_padding*2;
        rl_coords room_loc;
        room_loc.x = generator(loc.x + room_padding, loc.x + width - room_width - room_padding);
        room_loc.y = generator(loc.y + room_padding, loc.y + height - room_height - room_padding);

        for (int x = room_loc.x; x < room_loc.x + room_width; ++x) {
            for (int y = room_loc.y; y < room_loc.y + room_height; ++y) {
                if (x == room_loc.x || x == room_loc.x + room_width - 1 ||
                    y == room_loc.y || y == room_loc.y + room_height - 1
                ) {
                    // set sides of room to walls
                    rl_set_wall(map, (rl_coords){ x, y });
                } else {
                    rl_set_passable(map, (rl_coords){ x, y });
                }
            }
        }
    }

    if (corridor_algorithm)
        corridor_algorithm(map, generator, root);

    return map;
}

void connect_corridors_to_siblings(rl_map *map, rl_generator_f generator, rl_bsp *root)
{
    int max_adjacent_doors = 1;

    // populate room data map
    unsigned int map_width = rl_get_map_width(map);
    unsigned int map_height = rl_get_map_height(map);
    rl_map *room_map = rl_create_map(map_width, map_height);
    for (int x = 0; x < map_width; ++x) {
        for (int y = 0; y < map_height; ++y) {
            if (rl_is_wall(map, (rl_coords){ x, y }))
                rl_set_wall(room_map, (rl_coords){ x, y });
            if (rl_is_passable(map, (rl_coords){ x, y }))
                rl_set_passable(room_map, (rl_coords){ x, y });
        }
    }

    // generate corridors
    rl_queue *nodes = rl_get_bsp_nodes(root);
    rl_queue *cur;
    rl_bsp *node;
    while (node = rl_pop(&nodes)) {
        rl_coords node_loc = rl_get_bsp_loc(node);
        unsigned int node_width = rl_get_bsp_width(node);
        unsigned int node_height = rl_get_bsp_height(node);

        rl_bsp *sibling = rl_get_bsp_sibling(node);
        if (sibling == NULL) continue;
        rl_coords sibling_loc = rl_get_bsp_loc(sibling);
        unsigned int sibling_width = rl_get_bsp_width(sibling);
        unsigned int sibling_height = rl_get_bsp_height(sibling);

        // ensure sibling corridor hasn't already been carved
        cur = nodes;
        rl_bsp *target = NULL;
        while (cur) {
            target = rl_peek(cur);
            if (target == sibling) break;
            cur = cur->next;
        }
        if (target != sibling)
            continue; // sibling not found - corridor already carved

        // pick a random passable point within current node and
        // random passable point within sibling node, and connect them
        rl_coords start;
        rl_coords end;
        do {
            start = (rl_coords) {
                generator(node_loc.x, node_loc.x + node_width - 1),
                generator(node_loc.y, node_loc.y + node_height - 1)
            };
            end = (rl_coords) {
                generator(sibling_loc.x, sibling_loc.x + sibling_width - 1),
                generator(sibling_loc.y, sibling_loc.y + sibling_height - 1)
            };
        } while (!is_diggable(map, room_map, start, end));

        rl_path *path = rl_get_line_manhattan(start, end);
        rl_coords *coords;
        while (coords = rl_walk_path(path)) {
            int adjacent_doors = 0;

            // if wall, make sure we haven't hit adjacent door limit
            if (rl_is_wall(room_map, *coords)) {
                // find side of wall we're on
                if (rl_is_wall(room_map, (rl_coords){coords->x + 1, coords->y}) ||
                    rl_is_wall(room_map, (rl_coords){coords->x - 1, coords->y})
                ) {
                    if (rl_is_passable(map, (rl_coords){coords->x + 1, coords->y}))
                        adjacent_doors++;
                    if (rl_is_passable(map, (rl_coords){coords->x - 1, coords->y}))
                        adjacent_doors++;
                }
                if (rl_is_wall(room_map, (rl_coords){coords->x, coords->y + 1}) ||
                    rl_is_wall(room_map, (rl_coords){coords->x, coords->y - 1})
                ) {
                    if (rl_is_passable(map, (rl_coords){coords->x, coords->y + 1}))
                        adjacent_doors++;
                    if (rl_is_passable(map, (rl_coords){coords->x, coords->y - 1}))
                        adjacent_doors++;
                }
            }

            if (adjacent_doors < max_adjacent_doors)
                rl_set_passable(map, *coords);
        }
        rl_clear_path(path);
    }

    rl_free_map(room_map);
}

int is_corner(rl_map *map, rl_coords loc)
{
    if (!rl_is_wall(map, loc))
        return 0;

    if (rl_is_wall(map, (rl_coords){ loc.x + 1, loc.y }) && 
        (rl_is_wall(map, (rl_coords){ loc.x, loc.y + 1 }) || rl_is_wall(map, (rl_coords){ loc.x, loc.y - 1 }))
    ) {
        return 1;
    }

    if (rl_is_wall(map, (rl_coords){ loc.x - 1, loc.y }) && 
        (rl_is_wall(map, (rl_coords){ loc.x, loc.y + 1 }) || rl_is_wall(map, (rl_coords){ loc.x, loc.y - 1 }))
    ) {
        return 1;
    }

    return 0;
}

int is_diggable(rl_map *map, rl_map *room_map, rl_coords start, rl_coords end)
{
    int in_start = 1;
    int in_end = 0;

    // start & end can only be walls that *aren't* corners or passable tiles
    if ((rl_is_wall(room_map, start) && is_corner(room_map, start)) || !rl_is_passable(map, start))
        return 0;
    if ((rl_is_wall(room_map, end) && is_corner(room_map, end)) || !rl_is_passable(map, end))
        return 0;

    // ensure we don't go through any corners on the way
    rl_path *path = rl_get_line_manhattan(start, end);
    rl_coords *coords;
    while (coords = rl_walk_path(path)) {
        if (is_corner(room_map, *coords)) return 0;
    }
    rl_clear_path(path);

    return 1;
}
