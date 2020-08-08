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
                    rl_set_tile(map, (rl_coords){ x, y }, RL_TILE_WALL);
                } else {
                    rl_set_tile(map, (rl_coords){ x, y }, RL_TILE_ROOM);
                }
            }
        }
    }

    if (corridor_algorithm)
        corridor_algorithm(map, generator, root);

    return map;
}

int is_diggable(rl_map *map, rl_coords start, rl_coords end);
void connect_corridors_to_random_siblings(rl_map *map, rl_generator_f generator, rl_bsp *root)
{
    if (map == NULL || root == NULL) return;

    int max_adjacent_doors = 1;
    unsigned int map_width = rl_get_map_width(map);
    unsigned int map_height = rl_get_map_height(map);

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
        } while (!is_diggable(map, start, end));

        rl_path *path = rl_get_line_manhattan(start, end);
        rl_coords *coords;
        while (coords = rl_walk_path(path)) {
            int adjacent_doors = 0;

            // make sure we haven't hit adjacent door limit
            if (rl_is_wall(map, *coords)) {
                if (rl_is_doorway(map, (rl_coords){coords->x + 1, coords->y}))
                    adjacent_doors++;
                if (rl_is_doorway(map, (rl_coords){coords->x - 1, coords->y}))
                    adjacent_doors++;
                if (rl_is_doorway(map, (rl_coords){coords->x, coords->y + 1}))
                    adjacent_doors++;
                if (rl_is_doorway(map, (rl_coords){coords->x, coords->y - 1}))
                    adjacent_doors++;
            }

            if (adjacent_doors < max_adjacent_doors) {
                if (rl_is_wall(map, *coords))
                    rl_set_tile(map, *coords, RL_TILE_DOORWAY);
                else if (!rl_is_passable(map, *coords))
                    rl_set_tile(map, *coords, RL_TILE_PASSAGE);
            }
        }
        rl_clear_path(path);
    }
}

rl_path *dig_path(rl_map *map, rl_bsp *node, rl_bsp *sibling);
void connect_corridors_to_closest_siblings(rl_map *map, rl_generator_f generator, rl_bsp *root)
{
    if (map == NULL || root == NULL) return;

    int max_adjacent_doors = 1;

    // populate room data map
    unsigned int map_width = rl_get_map_width(map);
    unsigned int map_height = rl_get_map_height(map);

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

        rl_path *path = dig_path(map, node, sibling);
        rl_coords *coords;
        while (coords = rl_walk_path(path)) {
            if (rl_is_wall(map, *coords))
                rl_set_tile(map, *coords, RL_TILE_DOORWAY);
            else
                rl_set_tile(map, *coords, RL_TILE_PASSAGE);
        }
        rl_clear_path(path);
    }
}

int is_corner(rl_map *map, rl_coords loc)
{
    if (!rl_is_wall(map, loc) && !rl_is_doorway(map, loc))
        return 0;

    rl_tile tx1 = rl_get_tile(map, (rl_coords){ loc.x + 1, loc.y });
    rl_tile tx2 = rl_get_tile(map, (rl_coords){ loc.x - 1, loc.y });
    rl_tile ty1 = rl_get_tile(map, (rl_coords){ loc.x, loc.y + 1 });
    rl_tile ty2 = rl_get_tile(map, (rl_coords){ loc.x, loc.y - 1 });

    if ((tx1 == RL_TILE_WALL || tx1 == RL_TILE_DOORWAY) &&
        ((ty1 == RL_TILE_WALL || ty1 == RL_TILE_DOORWAY) || (ty2 == RL_TILE_WALL || ty2 == RL_TILE_DOORWAY))
    ) {
        return 1;
    }

    if ((tx2 == RL_TILE_WALL || tx2 == RL_TILE_DOORWAY) &&
        ((ty1 == RL_TILE_WALL || ty1 == RL_TILE_DOORWAY) || (ty2 == RL_TILE_WALL || ty2 == RL_TILE_DOORWAY))
    ) {
        return 1;
    }

    return 0;
}

int is_diggable(rl_map *map, rl_coords start, rl_coords end)
{
    // start & end can only be walls that *aren't* corners or passable tiles
    if (!rl_is_passable(map, start))
        return 0;
    if (!rl_is_passable(map, end))
        return 0;

    // ensure we don't go through any corners on the way
    rl_path *path = rl_get_line_manhattan(start, end);
    rl_coords *coords;
    while (coords = rl_walk_path(path)) {
        if (is_corner(map, *coords)) return 0;
    }
    rl_clear_path(path);

    return 1;
}

rl_path *dig_path(rl_map *map, rl_bsp *node, rl_bsp *sibling)
{
    if (map == NULL || node == NULL || sibling == NULL)
        return NULL;

    rl_coords node_loc = rl_get_bsp_loc(node);
    unsigned int node_width = rl_get_bsp_width(node);
    unsigned int node_height = rl_get_bsp_height(node);

    rl_coords sibling_loc = rl_get_bsp_loc(sibling);
    unsigned int sibling_width = rl_get_bsp_width(sibling);
    unsigned int sibling_height = rl_get_bsp_height(sibling);

    // find center point between nodes
    rl_coords center = {0};
    if (node_loc.x > sibling_loc.x) {
        center.x = node_loc.x;
        center.y += node_height/2;
    }
    if (node_loc.x < sibling_loc.x) {
        center.x = sibling_loc.x;
        center.y += sibling_height/2;
    }
    if (node_loc.y > sibling_loc.y) {
        center.y = node_loc.y;
        center.x += node_width/2;
    }
    if (node_loc.y < sibling_loc.y) {
        center.y = sibling_loc.y;
        center.x += sibling_width/2;
    }
    assert("BSP center cannot be found" && center.x && center.y);

    // find closest points to center for both nodes
    rl_queue *queue = NULL;
    for (int x = node_loc.x; x < node_loc.x + node_width; ++x) {
        for (int y = node_loc.y; y < node_loc.y + node_height; ++y) {
            // start & end can only be walls that *aren't* corners or passable tiles
            if (!rl_is_passable(map, (rl_coords){x,y}))
                continue;

            rl_coords *loc = malloc(sizeof(rl_coords));
            loc->x = x;
            loc->y = y;
            rl_push(&queue, loc, manhattan_distance(*loc, center));
        }
    }
    assert("BSP path cannot be dug" && queue);
    rl_coords *from;
    rl_coords to = center;
    while ((from = rl_pop(&queue))) {
        for (int x = sibling_loc.x; x < sibling_loc.x + sibling_width; ++x) {
            for (int y = sibling_loc.y; y < sibling_loc.y + sibling_height; ++y) {
                // start & end can only be walls that *aren't* corners or passable tiles
                if (!rl_is_passable(map, (rl_coords){x,y}) ||
                    !is_diggable(map, *from, (rl_coords){x,y})
                ) {
                    continue;
                }

                if ((to.x == center.x && to.y == center.y) ||
                    manhattan_distance(*from, to) > manhattan_distance(*from, (rl_coords){x,y})
                ) {
                    to = (rl_coords){x,y};
                }
            }
        }

        if (!(to.x == center.x && to.y == center.y) && is_diggable(map, *from, to))
            break;
        free(from);
    }
    assert("BSP path cannot be dug" && !(to.x == center.x && to.y == center.y) && from);

    rl_coords *loc;
    while ((loc = rl_pop(&queue))) {
        free(loc);
    }

    return rl_get_line_manhattan(*from, to);
}
