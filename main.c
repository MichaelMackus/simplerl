#include "draw.h"
#include "game.h"
#include "map.h"
#include "message.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define ERROR_OOM 1
#define ERROR_INIT 2

int main()
{
    // initialize curses
    if (!init()) {
        return ERROR_INIT;
    }

    // initialize dungeon
    Dungeon *dungeon = create_dungeon();
    if (dungeon == NULL)
        return ERROR_OOM;

    // allocate messages array
    const char **messages;
    messages = malloc(sizeof(char) * MAX_MESSAGE_LENGTH * MAX_MESSAGES);
    if (messages == NULL)
        return ERROR_OOM;

    init_game(dungeon, messages);

    int result = GAME_PLAYING;
    while (result == GAME_PLAYING)
    {
        // render & update curses
        update(dungeon, messages);

        // trigger gameloop
        result = gameloop(dungeon, messages);
    }

    // de-initialize curses
    deinit();

    if (result == GAME_WON)
        printf("You won!\n");
    else if (result == GAME_QUIT)
        printf("You quit.\n");
    else if (result == GAME_DEATH)
        printf("Oh no, you died :(\n");
    else if (result == GAME_OOM)
        return ERROR_OOM;
    else
        printf("Undefined game loop result...\n");

    return 0;
}
