#include "tile.h"

char tile_symbol(Tile tile)
{
    switch (tile.type)
    {
        case TILE_FLOOR:
            return '.';
        case TILE_CAVERN:
            return '#';
        case TILE_WALL:
            return '-';
        case TILE_WALL_SIDE:
            return '|';
        case TILE_STAIR_DOWN:
            return '>';
        case TILE_STAIR_UP:
            return '<';
        case TILE_DOOR:
            return '+';
        case TILE_DOOR_OPEN:
            return '_'; // TODO need better tile
        default:
            return ' ';
    }
}

int is_passable(Tile tile)
{
    if (tile.type == TILE_DOOR || tile.type == TILE_WALL || tile.type == TILE_WALL_SIDE || tile.type == TILE_NONE)
        return 0; //impassable
    else
        return 1; //passable
}

Tile create_tile(int tileType)
{
    Tile t;
    t.type = tileType;

    return t;
}
