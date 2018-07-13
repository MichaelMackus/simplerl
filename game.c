#include "game.h"

#include <ncurses.h>

int gameloop(Dungeon *dungeon, const char **messages)
{
    Mob *player = dungeon->player;

    // handle input
    char ch = getch();
    switch (ch)
    {
        case 'q':
            return GAME_QUIT;
        case 'h':
            --player->x;
            break;
        case 'l':
            ++player->x;
            break;
        case 'j':
            ++player->y;
            break;
        case 'k':
            --player->y;
            break;
    }

    return GAME_PLAYING;
}
