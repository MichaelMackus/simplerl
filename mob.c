#include "mob.h"
#include "path.h"
#include <stdlib.h>

Mob *enemy(unsigned int hp, unsigned int minDamage, unsigned int maxDamage, char symbol);
Mob *createMob(int depth)
{
    // difficulty ranges
    //
    // level 1: 1 - 2
    // level 2: 1 - 4
    // level 3: 2 - 6
    // level 5: 2 - 8
    // level 7: 4 - 12
    // level 10: 5 - 16
    int difficulty = (rand() % (depth+1)) + (depth / 2);

    // TODO need to scale difficulty more later (i.e. hobgoblins in alte dungeon...)
    if (difficulty == 1)
        return enemy(4, 1, 2, 'r'); // rat
    if (difficulty == 2)
        return enemy(4, 2, 3, 'k'); // kobold
    if (difficulty == 3)
        return enemy(5, 3, 5, 'g'); // goblin
    if (difficulty <= 5)
        return enemy(5, 3, 5, 'o'); // orc
    if (difficulty <= 7)
        return enemy(7, 5, 7, 'h'); // hobgoblin
    if (difficulty <= 9)
        return enemy(10, 6, 9, 'O'); // ogre
    if (difficulty <= 11)
        return enemy(12, 7, 10, 'd'); // drake
    if (difficulty <= 13)
    {
        Mob *m = enemy(12, 7, 10, 'H'); // mind flayer

        // OOM check
        if (m == NULL)
            return NULL;

        m->type = MOB_MIND_FLAYER;
        // TODO drain effect

        return m;
    }
    if (difficulty >= 15)
    {
        Mob *m = enemy(20, 10, 15, 'D'); // dragon

        // OOM check
        if (m == NULL)
            return NULL;

        m->type = MOB_DRAGON;
        // TODO breath effects

        return m;
    }
}

// try to attack x, y
// if no mob found at x, y do nothing
int attack(Mob *attacker, Mob *target)
{
    if (attacker == NULL || target == NULL)
        return 0;

    // calculate damage based on attacker's stats
    // TODO add simple roll function
    int damage = rand() % (attacker->maxDamage - attacker->minDamage + 1)  +  attacker->minDamage;

    target->hp -= damage;

    return damage;
}

Mob *enemy(unsigned int hp, unsigned int minDamage, unsigned int maxDamage, char symbol)
{
    Mob *m;
    m = malloc(sizeof(Mob));

    m->hp = hp;
    m->maxHP = hp;
    m->minDamage = minDamage;
    m->maxDamage = maxDamage;
    m->symbol = symbol;
    m->type = MOB_ENEMY;
    m->items = NULL;

    return m;
}

int can_smell(Mob *mob, Mob *player, Tile **tiles)
{
    return 0;
}

// TODO fix memory leak here
int can_see(const Mob *mob, int y, int x, Tile **tiles)
{
    int ret = 0;

    // only show 1 cavern at a time
    if (tiles[y][x].type == TILE_CAVERN)
    {
        const Coords **line = get_line(xy(mob->x, mob->y), xy(x, y));
        const Coords **current = line;
        while (*current != NULL)
        {
            // if the line ends at the point we're looking at, we can see it!
            if ((*current)->x == x && (*current)->y == y)
            {
                ret = 1; // success!

                break;
            }

            // if the current coord blocks the view, we can't see
            int type = tiles[(*current)->y][(*current)->x].type;
            if (type == TILE_NONE || type == TILE_WALL_SIDE || type == TILE_WALL || type == TILE_CAVERN)
                break;

            ++current;
        }
        free_path(line);
    }
    else
    {
        const Coords **line = get_line(xy(mob->x, mob->y), xy(x, y));
        const Coords **current = line;
        while (*current != NULL)
        {
            // if the line ends at the point we're looking at, we can see it!
            if ((*current)->x == x && (*current)->y == y)
            {
                ret = 1; // success!

                break;
            }

            // if the current coord blocks the view, we can't see
            int type = tiles[(*current)->y][(*current)->x].type;
            if (type == TILE_NONE || type == TILE_WALL_SIDE || type == TILE_WALL || type == TILE_CAVERN)
                break;

            ++current;
        }
        free_path(line);
    }

    return ret;
}
