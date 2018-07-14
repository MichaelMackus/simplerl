#include "tile.h"

char tile_symbol(Tile tile)
{
    switch (tile.type)
    {
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
            return ' '; // TODO need better tile
        default:
            return ' ';
    }
}
