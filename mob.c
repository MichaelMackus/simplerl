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

    Mob *m;

    // TODO need to scale difficulty more later (i.e. hobgoblins in alte dungeon...)
    if (difficulty == 1)
        m = enemy(4, 1, 2, 'r'); // rat
    else if (difficulty == 2)
        m = enemy(4, 2, 3, 'k'); // kobold
    else if (difficulty == 3)
        m = enemy(5, 3, 5, 'g'); // goblin
    else if (difficulty <= 5)
        m = enemy(5, 3, 5, 'o'); // orc
    else if (difficulty <= 7)
        m = enemy(7, 5, 7, 'h'); // hobgoblin
    else if (difficulty <= 9)
        m = enemy(10, 6, 9, 'O'); // ogre
    else if (difficulty <= 11)
        m = enemy(12, 7, 10, 'd'); // drake
    else if (difficulty <= 13)
    {
        m = enemy(12, 7, 10, 'H'); // mind flayer

        // OOM check
        if (m == NULL)
            return NULL;

        m->type = MOB_MIND_FLAYER;
        // TODO drain effect
    }
    else if (difficulty >= 15)
    {
        m = enemy(20, 10, 15, 'D'); // dragon

        // OOM check
        if (m == NULL)
            return NULL;

        m->type = MOB_DRAGON;
        // TODO breath effects
    }

    m->difficulty = difficulty;
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

int can_see(const Mob *mob, Coords coords, Tile **tiles)
{
    int ret = 0;

    // only show 1 cavern at a time
    if (tiles[coords.y][coords.x].type == TILE_CAVERN)
    {
        const Coords **line = get_line(mob->coords, coords);
        const Coords **current = line;
        while (*current != NULL)
        {
            // if the line ends at the point we're looking at, we can see it!
            if ((*current)->x == coords.x && (*current)->y == coords.y)
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
        const Coords **line = get_line(mob->coords, coords);
        const Coords **current = line;
        while (*current != NULL)
        {
            // if the line ends at the point we're looking at, we can see it!
            if ((*current)->x == coords.x && (*current)->y == coords.y)
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
