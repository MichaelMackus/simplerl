#include "game.h"
#include "message.h"

#include <ncurses.h>

int alert_depth(Dungeon *dungeon, const char **messages);

int init_game(Dungeon *dungeon, const char **messages)
{
    if (dungeon->player == NULL || dungeon->level == NULL || messages == NULL)
    {
        // simple error case
        return 1;
    }

    dungeon->player->x = 40;
    dungeon->player->y = 15;

    // alert level depth on game start
    alert_depth(dungeon, messages);

    return 0;
}

uint8_t gameloop(Dungeon *dungeon, const char **messages)
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
        case '>':
            if (!increase_depth(dungeon))
                // if error, assume we won
                return GAME_WON;
            alert_depth(dungeon, messages);
            break;
        case '<':
            if (!decrease_depth(dungeon))
                // if error, assume we escaped
                return GAME_QUIT;
            alert_depth(dungeon, messages);
            break;
        case 'D':
            return GAME_DEATH;
        case 'W':
            return GAME_WON;
        case 'E':
            return ERROR_OOM;
    }

    return GAME_PLAYING;
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

// insert the current dungeon depth to messages array
int alert_depth(Dungeon *dungeon, const char **messages)
{
    // allocate memory for message
    const char *message = create_message("Current level: %d", dungeon->level->depth);
    if (message == NULL)
        return 1;

    // insert the message
    insert_message(message, messages);

    return 0;
}
